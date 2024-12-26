#include "../parser/toml_parser.hpp"
#include "protocols.h"

std::string DIRECTORY = std::string(parseField("library", "directory"));

// Predefined credentials (hashed password with salt)
const std::string USERNAME = std::string(parseField("ftp", "username"));
const std::string SALT = std::string(parseField("ftp", "salt"));
/* Server only has the password hash; Client will have to give the password in and the server will compute the hash of salt+password and compare it with the given password hash in config.toml */
const std::string PASSWORD_HASH = std::string(parseField("ftp", "password_hash"));

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
        send_protocol_message(ssl, STATUS_AUTH_FAILED);
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
        send_protocol_message(ssl, STATUS_AUTH_FAILED);
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
        send_protocol_message(ssl, STATUS_AUTH_FAILED);
        return false;
    }

    send_protocol_message(ssl, STATUS_AUTH_SUCCESS);
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
        send_protocol_message(ssl, STATUS_FILE_NOT_FOUND);
        return;
    }

    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        send_protocol_message(ssl, STATUS_FILE_NOT_FOUND);
        return;
    }

    // Calculate file checksum
    std::ostringstream file_data;
    char buffer[BUFFER_SIZE];
    while (file.read(buffer, sizeof(buffer))) {
        file_data.write(buffer, file.gcount());
    }
    std::string file_content = file_data.str();
    std::string checksum = sha256(file_content);

    // Send checksum
    send_protocol_message(ssl, STATUS_CHECKSUM_VERIFY);
    SSL_write(ssl, (checksum).c_str(), checksum.size());
    SSL_write(ssl, "\n", 1);

    // Confirm transfer
    send_protocol_message(ssl, STATUS_FILE_TRANSFER);

    file.clear();
    file.seekg(0, std::ios::beg);
    while (!file.eof()) {
        file.read(buffer, sizeof(buffer));
        int bytes_read = file.gcount();
        if (SSL_write(ssl, buffer, bytes_read) <= 0) {
            break;
        }
    }

    send_protocol_message(ssl, STATUS_TRANSFER_COMPLETE);
    file.close();
}

// Sanitize file paths
std::string sanitize_path(const std::string &file_name) {
    std::string safe_path = file_name;

    // Prevent directory traversal
    if (safe_path.find("..") != std::string::npos) {
        return "";
    }

    // Add more sanitization rules if necessary
    return safe_path;
}

void handle_client(SSL *ssl, const std::string &client_ip) {
    char buffer[BUFFER_SIZE] = {0};
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = SSL_read(ssl, buffer, BUFFER_SIZE);
        if (bytes_received <= 0) {
            std::cerr << "[SERVER] Client disconnected or error occurred." << std::endl;
            break;
        }

        std::string command(buffer);
        std::istringstream iss(command);
        std::string action, argument;
        iss >> action;

        // For 'GET' and 'LIST', read the entire line (file name with spaces)
        if (action == GET_REQUEST || action == LIST_REQUEST) {
            std::getline(iss, argument); // Read the rest of the line as the file name (with spaces)
            if (!argument.empty() && argument[0] == ' ') {
                argument = argument.substr(1); // Remove the leading space
            }
        }

        if (action == AUTH_REQUEST) {
            if (!authenticate_client(ssl, client_ip)) {
                break;
            }
        } else if (action == LIST_REQUEST) {
            std::string files = list_files();
            send_protocol_message(ssl, STATUS_LISTING_FILES);
            SSL_write(ssl, (files).c_str(), files.size() + 40);
            send_protocol_message(ssl, STATUS_TRANSFER_COMPLETE);
        } else if (action == GET_REQUEST) {
            if (argument.empty()) {
                send_protocol_message(ssl, STATUS_MISSING_FILE_NAME);
                continue;
            }
            // Sanitize the file path and send the file
            std::string file_path = sanitize_path(DIRECTORY + argument);
            if (!file_path.empty()) {
                send_file(ssl, argument);
            } else {
                send_protocol_message(ssl, STATUS_FILE_NOT_FOUND);
            }
        } else if (action == QUIT_REQUEST) {
            send_protocol_message(ssl, STATUS_GOODBYE);
            break;
        } else {
            send_protocol_message(ssl, STATUS_UNKNOWN_COMMAND);
        }
    }
    SSL_shutdown(ssl);
    SSL_free(ssl);
}

int main() {
    std::cout << "[SERVER] Initializing FTP server for " << DIRECTORY << std::endl;
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
        perror("[SERVER] Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("[SERVER] Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "--> Server is listening on port " << PORT << std::endl;

    while (true) {
        struct sockaddr_in client_addr = {};
        socklen_t addr_len = sizeof(client_addr);
        int client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("[SERVER] Accept failed");
            continue;
        }

        std::string client_ip = inet_ntoa(client_addr.sin_addr);
        std::cout << "==> Connection established with " << client_ip << std::endl;

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
