#include "protocols.h"
#include <termios.h>
#include <unistd.h>

// Credentials - only username and salt is stored, password will be input at runtime
// TODO:: Removing hardcoded vars like USERNAME and SALT
const std::string USERNAME = "client";
const std::string SALT = "random_salt_value"; // This should match the server's salt basically (found in config.toml)

std::string sha256(const std::string& str) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(str.c_str()), str.size(), hash);
    
    std::ostringstream oss;
    for(unsigned char byte : hash) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return oss.str();
}

SSL_CTX* initialize_ssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (!SSL_CTX_load_verify_locations(ctx, CA_CERT, nullptr)) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
    return ctx;
}

// Function to securely read password
std::string read_password() {
    std::string password;
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::getline(std::cin, password);

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << std::endl;
    return password;
}

bool authenticate(SSL* ssl) {
    char buffer[BUFFER_SIZE];
    
    // Send AUTH command
    if (SSL_write(ssl, AUTH_REQUEST, strlen(AUTH_REQUEST)) <= 0) {
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
    std::string auth_failed_str = get_error_message(STATUS_AUTH_FAILED);
    if (challenge.find(auth_failed_str) != std::string::npos) {
        std::cout << challenge << std::endl;
        return false;
    }

    // Get password from user
    std::cout << "Enter password: ";
    std::string password = read_password();

    // First compute the salted password hash
    std::string salted_password_hash = sha256(SALT + password);

    std::cout << "[CLIENT] " << salted_password_hash << std::endl;
    
    // Then compute the challenge response using the salted password hash
    std::string response = sha256(challenge + salted_password_hash);

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
    return result.find(get_error_message(STATUS_AUTH_SUCCESS)) != std::string::npos;
}

bool receive_file(SSL* ssl, const std::string& filename) {
    char buffer[BUFFER_SIZE];
    std::string command = GET_REQUEST + std::string(" ") + filename;
    
    if (SSL_write(ssl, command.c_str(), command.length()) <= 0) {
        std::cerr << "Failed to send GET command" << std::endl;
        return false;
    }

    memset(buffer, 0, BUFFER_SIZE);
    if (SSL_read(ssl, buffer, BUFFER_SIZE) <= 0) {
        std::cerr << "Failed to receive response" << std::endl;
        return false;
    }
    
    std::string response(buffer);
    if (response.find(get_error_message(STATUS_FILE_NOT_FOUND)) != std::string::npos) {
        std::cout << response;
        return false;
    }

    std::string checksum;
    if (response.find("File checksum:") != std::string::npos) {
        size_t start = response.find(": ") + 2;
        size_t end = response.find("\n", start);
        checksum = response.substr(start, end - start);
    }

    // Open file for writing
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing" << std::endl;
        return false;
    }

    std::ostringstream received_data;
    bool started_transfer = false;  // Flag to check if file transfer has started

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = SSL_read(ssl, buffer, BUFFER_SIZE);
        if (bytes <= 0) break;
        
        std::string chunk(buffer, bytes);

        // Skip the STATUS_CHECKSUM_VERIFY and checksum lines
        if (chunk.find(get_error_message(STATUS_CHECKSUM_VERIFY)) != std::string::npos) {
            continue; // Skip checksum verify message
        }

        if (!started_transfer) {
            // Wait until we receive the STATUS_FILE_TRANSFER message before writing data
            if (chunk.find(get_error_message(STATUS_FILE_TRANSFER)) != std::string::npos) {
                started_transfer = true;  // File transfer has started
                continue;  // Skip the STATUS_FILE_TRANSFER message
            }
            continue; // Skip any other data until transfer starts
        }

        // Break on transfer complete message
        if (chunk.find(get_error_message(STATUS_TRANSFER_COMPLETE)) != std::string::npos) {
            break;
        }

        // Write data to the file once transfer has started
        file.write(buffer, bytes);
        received_data.write(buffer, bytes);
    }

    file.close();

    std::string received_checksum = sha256(received_data.str());
    if (!checksum.empty() && received_checksum != checksum) {        
        std::cout << "[DEBUG] Received checksum from server: " << checksum << std::endl;
        std::cout << "[DEBUG] Calculated checksum from received data: " << received_checksum << std::endl;
        std::cout << "[DEBUG] Removing possible corrupted file..." << std::endl;
        std::filesystem::remove(filename);
        return false;
    }

    std::cout << "File received and verified successfully" << std::endl;
    return true;
}

void list_files(SSL* ssl) {
    if (SSL_write(ssl, LIST_REQUEST, strlen(LIST_REQUEST)) <= 0) {
        std::cerr << "Failed to send LIST command" << std::endl;
        return;
    }

    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = SSL_read(ssl, buffer, BUFFER_SIZE);
        if (bytes <= 0) break;
        
        std::string response(buffer);
        std::cout << response;
        if (response.find(get_error_message(STATUS_TRANSFER_COMPLETE)) != std::string::npos) {
            break;
        }
    }
}

void print_help() {
    std::cout << "\nAvailable commands:\n"
              << "  LIST            - List available files\n"
              << "  GET <filename>  - Download a file\n"
              << "  HELP           - Show this help message\n"
              << "  QUIT           - Exit the program\n";
}

int main() {
    SSL_CTX* ctx = initialize_ssl();
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);
    
    if (SSL_connect(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

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

    if (!authenticate(ssl)) {
        std::cerr << "Authentication failed" << std::endl;
        goto cleanup;
    }

    std::cout << "\nAuthentication successful! Type HELP for available commands." << std::endl;

    while (true) {
        std::string command;
        std::cout << "\nftp> ";
        std::getline(std::cin, command);

        if (command == "QUIT") {
            SSL_write(ssl, command.c_str(), command.length());
            break;
        } else if (command == "LIST") {
            list_files(ssl);
        } else if (command == "HELP") {
            print_help();
        } else if (command.substr(0, 3) == "GET") {
            std::string filename = command.substr(4);
            if (filename.empty()) {
                std::cout << "Please specify a filename" << std::endl;
                continue;
            }
            receive_file(ssl, filename);
        } else {
            std::cout << "Unknown command. Type 'HELP' for available commands." << std::endl;
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
