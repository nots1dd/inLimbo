#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <string>

#define SERVER_IP "127.0.0.1" // Loopback addr just to test client and server on same system, ideally you should give the IP addr of the server
#define PORT 12345
#define BUFFER_SIZE 1024

#define CA_CERT "server_cert.pem"  // Certificate Authority certificate

// Credentials - matching exactly with server
const std::string USERNAME = "client";
const std::string PASSWORD_HASH = "f8b9e6974d80c42ebc6d2cdbff240220e78a19f4915796df51f856cb43229e43";

// Utility function to compute SHA-256 hash
std::string sha256(const std::string& str) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(str.c_str()), str.size(), hash);
    
    std::ostringstream oss;
    for(unsigned char byte : hash) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return oss.str();
}

// Initialize OpenSSL
SSL_CTX* initialize_ssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Load trusted CA certificate
    if (!SSL_CTX_load_verify_locations(ctx, CA_CERT, nullptr)) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Require server certificate verification
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);

    return ctx;
}

// Authenticate with server
bool authenticate(SSL* ssl) {
    char buffer[BUFFER_SIZE];
    
    // Send AUTH command first
    const char* auth_cmd = "AUTH";
    if (SSL_write(ssl, auth_cmd, strlen(auth_cmd)) <= 0) {
        std::cerr << "Failed to send AUTH command" << std::endl;
        return false;
    }

    // Send username
    if (SSL_write(ssl, USERNAME.c_str(), USERNAME.length()) <= 0) {
        std::cerr << "Failed to send username" << std::endl;
        return false;
    }

    // Receive challenge
    memset(buffer, 0, BUFFER_SIZE);
    if (SSL_read(ssl, buffer, BUFFER_SIZE) <= 0) {
        std::cerr << "Failed to receive challenge" << std::endl;
        return false;
    }

    std::string challenge(buffer);
    if (challenge.find("430") == 0) {
        std::cout << challenge << std::endl;
        return false;
    }

    // Calculate response using server's expected hash
    std::string response = sha256(challenge + PASSWORD_HASH);

    // Send response
    if (SSL_write(ssl, response.c_str(), response.length()) <= 0) {
        std::cerr << "Failed to send response" << std::endl;
        return false;
    }

    // Receive authentication result
    memset(buffer, 0, BUFFER_SIZE);
    if (SSL_read(ssl, buffer, BUFFER_SIZE) <= 0) {
        std::cerr << "Failed to receive authentication result" << std::endl;
        return false;
    }

    std::string result(buffer);
    std::cout << result;
    return result.find("230") == 0;
}

// Receive file from server
bool receive_file(SSL* ssl, const std::string& filename) {
    char buffer[BUFFER_SIZE];
    std::string command = "GET " + filename;
    
    // Send GET command
    if (SSL_write(ssl, command.c_str(), command.length()) <= 0) {
        std::cerr << "Failed to send GET command" << std::endl;
        return false;
    }

    // Receive checksum
    memset(buffer, 0, BUFFER_SIZE);
    if (SSL_read(ssl, buffer, BUFFER_SIZE) <= 0) {
        std::cerr << "Failed to receive checksum" << std::endl;
        return false;
    }
    
    std::string response(buffer);
    if (response.find("550") == 0) {
        std::cout << response;
        return false;
    }

    std::string checksum = response.substr(response.find(": ") + 2);
    checksum = checksum.substr(0, checksum.find('\n'));

    // Receive transfer start confirmation
    memset(buffer, 0, BUFFER_SIZE);
    if (SSL_read(ssl, buffer, BUFFER_SIZE) <= 0) {
        std::cerr << "Failed to receive transfer confirmation" << std::endl;
        return false;
    }

    // Open file for writing
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing" << std::endl;
        return false;
    }

    // Receive file data
    std::ostringstream received_data;
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = SSL_read(ssl, buffer, BUFFER_SIZE);
        if (bytes <= 0) break;
        
        std::string chunk(buffer, bytes);
        if (chunk.find("226") == 0) break;
        
        file.write(buffer, bytes);
        received_data << std::string(buffer, bytes);
    }
    file.close();

    // Verify checksum
    std::string received_checksum = sha256(received_data.str());
    if (received_checksum != checksum) {
        std::cerr << "Checksum verification failed!" << std::endl;
        return false;
    }

    std::cout << "File received and verified successfully" << std::endl;
    return true;
}

// List files on server
void list_files(SSL* ssl) {
    char buffer[BUFFER_SIZE];
    const char* command = "LIST";
    
    if (SSL_write(ssl, command, strlen(command)) <= 0) {
        std::cerr << "Failed to send LIST command" << std::endl;
        return;
    }

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = SSL_read(ssl, buffer, BUFFER_SIZE);
        if (bytes <= 0) break;
        
        std::string response(buffer);
        std::cout << response;
        if (response.find("226") != std::string::npos) break;
    }
}

int main() {
    SSL_CTX* ctx = initialize_ssl();
    
    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address
    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Create SSL connection
    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);
    
    if (SSL_connect(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Verify server certificate
    X509* cert = SSL_get_peer_certificate(ssl);
    if (!cert) {
        std::cerr << "No certificate presented by server" << std::endl;
        exit(EXIT_FAILURE);
    }
    X509_free(cert);

    if (SSL_get_verify_result(ssl) != X509_V_OK) {
        std::cerr << "Server certificate verification failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Connected to server. Starting authentication..." << std::endl;

    // Authenticate
    if (!authenticate(ssl)) {
        std::cerr << "Authentication failed" << std::endl;
        goto cleanup;
    }

    // Main command loop
    while (true) {
        std::string command;
        std::cout << "\nEnter command (LIST, GET <filename>, or QUIT): ";
        std::getline(std::cin, command);

        if (command == "QUIT") {
            SSL_write(ssl, command.c_str(), command.length());
            break;
        } else if (command == "LIST") {
            list_files(ssl);
        } else if (command.substr(0, 3) == "GET") {
            std::string filename = command.substr(4);
            receive_file(ssl, filename);
        } else {
            std::cout << "Invalid command" << std::endl;
        }
    }

cleanup:
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sock);
    SSL_CTX_free(ctx);
    EVP_cleanup();

    return 0;
}
