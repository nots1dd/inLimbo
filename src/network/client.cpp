#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fstream>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

#define PORT 12345
#define BUFFER_SIZE 1024
#define HARDCODED_IP_ADDR "192.168.1.18"

// Predefined credentials
const std::string USERNAME = "client";
const std::string PASSWORD = "password";

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

// Authenticate with the server
bool authenticate_with_server(int sock) {
    char buffer[BUFFER_SIZE] = {0};

    // Send username
    send(sock, USERNAME.c_str(), USERNAME.size(), 0);

    // Receive challenge from server
    int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        std::cerr << "ERROR: Failed to receive challenge from server." << std::endl;
        return false;
    }
    std::string challenge(buffer, bytes_received);

    // Compute response: SHA256(challenge + hashed password)
    std::string password_hash = sha256(PASSWORD);
    std::string response = sha256(challenge + password_hash);

    // Send response to server
    send(sock, response.c_str(), response.size(), 0);

    // Receive authentication result
    memset(buffer, 0, BUFFER_SIZE);
    bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0 || std::string(buffer) != "AUTH_SUCCESS") {
        std::cout << response << " " << password_hash << std::endl;
        std::cout << std::string(buffer) << std::endl;
        std::cerr << "ERROR: Authentication failed." << std::endl;
        return false;
    }

    std::cout << "Authentication successful." << std::endl;
    return true;
}

void receive_file(int sock, const std::string& save_path) {
    std::ofstream file(save_path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "ERROR: Unable to create file " << save_path << std::endl;
        return;
    }

    char buffer[BUFFER_SIZE];
    int bytes_received;
    while ((bytes_received = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        file.write(buffer, bytes_received);
        if (bytes_received < BUFFER_SIZE) break; // End of file
    }
    file.close();
    std::cout << "File saved as: " << save_path << std::endl;
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert address to binary form
    if (inet_pton(AF_INET, HARDCODED_IP_ADDR, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    // Authenticate with the server
    if (!authenticate_with_server(sock)) {
        close(sock);
        return -1;
    }

    // Receive file list
    int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
    if (bytes_received > 0) {
        std::cout << "Available files:\n" << buffer << std::endl;
    } else {
        std::cerr << "ERROR: Failed to receive file list." << std::endl;
        close(sock);
        return -1;
    }

    // Request a file
    std::string file_request;
    std::cout << "Enter the name of the file to download: ";
    std::cin >> file_request;
    send(sock, file_request.c_str(), file_request.size(), 0);

    // Confirm file download
    bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
    if (bytes_received > 0) {
        std::cout << buffer << std::endl;
    } else {
        std::cerr << "ERROR: Failed to receive confirmation message." << std::endl;
        close(sock);
        return -1;
    }

    std::string response;
    std::cout << "Confirm download (yes/no): ";
    std::cin >> response;
    send(sock, response.c_str(), response.size(), 0);

    if (response == "yes") {
        std::string save_path = file_request;
        receive_file(sock, save_path);
    } else {
        std::cout << "File transfer cancelled." << std::endl;
    }

    close(sock);
    return 0;
}
