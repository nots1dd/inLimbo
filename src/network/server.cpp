#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <string>
#include <filesystem>
#include <random>

#define PORT 12345
#define BUFFER_SIZE 1024
#define DIRECTORY "/home/s1dd/misc/playground/shared_files/"

// Predefined credentials (hashed password)
const std::string USERNAME = "client";
const std::string PASSWORD_HASH = "5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8"; // Hash of "password"

// Utility function to compute SHA-256 hash
std::string sha256(const std::string& str) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(str.c_str()), str.size(), hash);

    std::ostringstream oss;
    for (unsigned char byte : hash) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return oss.str();
}

// Authenticate client
bool authenticate_client(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};

    // Receive username
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    std::string username(buffer);
    if (username != USERNAME) {
        send(client_socket, "AUTH_FAIL", 9, 0);
        return false;
    }

    // Generate a challenge (random nonce)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(100000, 999999);
    int challenge = dis(gen);
    std::string challenge_str = std::to_string(challenge);

    // Send the challenge to the client
    send(client_socket, challenge_str.c_str(), challenge_str.size(), 0);

    // Receive the response
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    std::string client_response(buffer);

    // Verify response (SHA-256(challenge + password))
    std::string expected_response = sha256(challenge_str + PASSWORD_HASH);
    if (client_response != expected_response) {
        send(client_socket, "AUTH_FAIL", 9, 0);
        return false;
    }

    send(client_socket, "AUTH_SUCCESS", 12, 0);
    return true;
}

// List files in the shared directory
std::string list_files() {
    std::ostringstream oss;
    for (const auto& entry : std::filesystem::directory_iterator(DIRECTORY)) {
        if (entry.is_regular_file()) {
            oss << entry.path().filename().string() << "\n";
        }
    }
    return oss.str();
}

// Send file
void send_file(int client_socket, const std::string& file_name) {
    std::string file_path = DIRECTORY + file_name;

    // Validate the file path to prevent directory traversal attacks
    if (file_path.find("..") != std::string::npos || !std::filesystem::exists(file_path)) {
        std::cerr << "Invalid file request: " << file_name << std::endl;
        const char* error_msg = "ERROR: File not found or invalid file path.";
        send(client_socket, error_msg, strlen(error_msg), 0);
        return;
    }

    // Ask for confirmation before sending the file
    const char* confirmation_msg = "CONFIRM_REQUEST";
    send(client_socket, confirmation_msg, strlen(confirmation_msg), 0);

    char confirmation_buffer[BUFFER_SIZE] = {0};
    recv(client_socket, confirmation_buffer, BUFFER_SIZE, 0);

    if (std::string(confirmation_buffer) != "yes") {
        std::cerr << "File transfer canceled by client." << std::endl;
        return;
    }

    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << file_name << std::endl;
        const char* error_msg = "ERROR: Could not open file.";
        send(client_socket, error_msg, strlen(error_msg), 0);
        return;
    }

    char buffer[BUFFER_SIZE];
    while (!file.eof()) {
        file.read(buffer, BUFFER_SIZE);
        int bytes_read = file.gcount();
        if (send(client_socket, buffer, bytes_read, 0) == -1) {
            std::cerr << "Error: Failed to send file " << file_name << std::endl;
            break;
        }
    }

    file.close();
    std::cout << "File " << file_name << " sent successfully." << std::endl;
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all network interfaces
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
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
        client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        std::cout << "Connection established with " << inet_ntoa(client_addr.sin_addr) << std::endl;

        if (!authenticate_client(client_socket)) {
            std::cerr << "Authentication failed for client." << std::endl;
            close(client_socket);
            continue;
        }

        // Send file list to client
        std::string files = list_files();
        send(client_socket, files.c_str(), files.size(), 0);

        // Receive file request
        char file_name[BUFFER_SIZE] = {0};
        int bytes_received = recv(client_socket, file_name, BUFFER_SIZE, 0);

        if (bytes_received <= 0) {
            std::cerr << "Error: Failed to receive file request." << std::endl;
            close(client_socket);
            continue;
        }

        std::cout << "Client requested file: " << file_name << std::endl;
        send_file(client_socket, file_name);

        close(client_socket);
    }

    close(server_fd);
    return 0;
}
