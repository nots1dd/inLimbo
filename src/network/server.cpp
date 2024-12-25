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
#include <filesystem>
#include <random>
#include <chrono>
#include <thread>
#include <unordered_map>

#define PORT 12345
#define BUFFER_SIZE 1024
#define DIRECTORY "/home/s1dd/misc/playground/shared_files/"
#define MAX_ATTEMPTS 3
#define ATTEMPT_DELAY 2000 // in ms

#define SERVER_CERT "server_cert.pem"
#define SERVER_KEY "server_key.pem"

// Predefined credentials (hashed password with salt)
const std::string USERNAME = "client";
const std::string SALT = "random_salt_value";
const std::string PASSWORD_HASH = "f8b9e6974d80c42ebc6d2cdbff240220e78a19f4915796df51f856cb43229e43"; // Hash of "random_salt_valuepassword" (must be some better way of doing this lol)

std::unordered_map<std::string, int> auth_attempts;

std::string sha256(const std::string &str) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char *>(str.c_str()), str.size(), hash);

    std::ostringstream oss;
    for (unsigned char byte : hash) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return oss.str();
}

SSL_CTX *initialize_ssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Load server certificate and private key
    if (SSL_CTX_use_certificate_file(ctx, SERVER_CERT, SSL_FILETYPE_PEM) <= 0 ||
        SSL_CTX_use_PrivateKey_file(ctx, SERVER_KEY, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

bool authenticate_client(SSL *ssl, const std::string &client_ip) {
    char buffer[BUFFER_SIZE] = {0};

    // Rate limiting for authentication attempts
    if (auth_attempts[client_ip] >= MAX_ATTEMPTS) {
        SSL_write(ssl, "430 Too many failed attempts\n", 29);
        std::this_thread::sleep_for(std::chrono::milliseconds(ATTEMPT_DELAY));
        return false;
    }

    // Receive username
    if (SSL_read(ssl, buffer, BUFFER_SIZE) <= 0) {
        return false;
    }
    std::string username(buffer);
    if (username != USERNAME) {
        auth_attempts[client_ip]++;
        SSL_write(ssl, "430 Authentication failed\n", 26);
        return false;
    }

    // Generate a challenge (random nonce)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(100000, 999999);
    int challenge = dis(gen);
    std::string challenge_str = std::to_string(challenge);

    // Send the challenge to the client
    SSL_write(ssl, challenge_str.c_str(), challenge_str.size());

    memset(buffer, 0, BUFFER_SIZE);
    if (SSL_read(ssl, buffer, BUFFER_SIZE) <= 0) {
        return false;
    }
    std::string client_response(buffer);

    // Verify response (SHA-256(challenge + salted password hash))
    std::string expected_response = sha256(challenge_str + PASSWORD_HASH);
    if (client_response != expected_response) {
        auth_attempts[client_ip]++;
        SSL_write(ssl, "430 Authentication failed\n", 26);
        return false;
    }

    SSL_write(ssl, "230 Authentication successful\n", 30);
    auth_attempts.erase(client_ip); // Reset attempts on success
    return true;
}

std::string list_files() {
    std::ostringstream oss;
    for (const auto &entry : std::filesystem::directory_iterator(DIRECTORY)) {
        if (entry.is_regular_file()) {
            oss << entry.path().filename().string() << "\n";
        }
    }
    return oss.str();
}

// Send file with checksum
void send_file(SSL *ssl, const std::string &file_name) {
    std::string file_path = DIRECTORY + file_name;

    if (file_path.find("..") != std::string::npos || !std::filesystem::exists(file_path)) {
        const char *error_msg = "550 File not found or invalid file path\n";
        SSL_write(ssl, error_msg, strlen(error_msg));
        return;
    }

    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        const char *error_msg = "550 Could not open file\n";
        SSL_write(ssl, error_msg, strlen(error_msg));
        return;
    }

    // Calculate file checksum (not working need to fix it)
    std::ostringstream file_data;
    char buffer[BUFFER_SIZE];
    while (file.read(buffer, sizeof(buffer))) {
        file_data.write(buffer, file.gcount());
    }
    std::string file_content = file_data.str();
    std::string checksum = sha256(file_content);

    // Send checksum
    SSL_write(ssl, ("150 File checksum: " + checksum + "\n").c_str(), checksum.size() + 23);

    // Confirm transfer
    SSL_write(ssl, "150 Starting file transfer\n", 27);

    file.clear();
    file.seekg(0, std::ios::beg);
    while (!file.eof()) {
        file.read(buffer, sizeof(buffer));
        int bytes_read = file.gcount();
        if (SSL_write(ssl, buffer, bytes_read) <= 0) {
            break;
        }
    }

    SSL_write(ssl, "226 Transfer complete\n", 23);
    file.close();
}

void handle_client(SSL *ssl, const std::string &client_ip) {
    char buffer[BUFFER_SIZE] = {0};
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = SSL_read(ssl, buffer, BUFFER_SIZE);
        if (bytes_received <= 0) {
            std::cerr << "Client disconnected or error occurred." << std::endl;
            break;
        }

        std::string command(buffer);
        std::istringstream iss(command);
        std::string action, argument;
        iss >> action;
        iss >> argument;

        if (action == "AUTH") {
            if (!authenticate_client(ssl, client_ip)) {
                break;
            }
        } else if (action == "LIST") {
            std::string files = list_files();
            SSL_write(ssl, ("150 Listing files:\n" + files + "226 Transfer complete\n").c_str(), files.size() + 40);
        } else if (action == "GET") {
            if (argument.empty()) {
                SSL_write(ssl, "400 Missing file name\n", 22);
                continue;
            }
            send_file(ssl, argument);
        } else if (action == "QUIT") {
            SSL_write(ssl, "221 Goodbye\n", 12);
            break;
        } else {
            SSL_write(ssl, "500 Unknown command\n", 21);
        }
    }
    SSL_shutdown(ssl);
    SSL_free(ssl);
}

int main() {
    SSL_CTX *ctx = initialize_ssl();

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is listening on port " << PORT << std::endl;

    while (true) {
        struct sockaddr_in client_addr = {};
        socklen_t addr_len = sizeof(client_addr);
        int client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        std::string client_ip = inet_ntoa(client_addr.sin_addr);
        std::cout << "Connection established with " << client_ip << std::endl;

        SSL *ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client_socket);

        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
            SSL_free(ssl);
            close(client_socket);
            continue;
        }

        handle_client(ssl, client_ip);
        close(client_socket);
    }

    close(server_fd);
    SSL_CTX_free(ctx);
    EVP_cleanup();

    return 0;
}
