#ifndef PROTOCOLS_H
#define PROTOCOLS_H

#include <arpa/inet.h>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/ssl.h>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>
#include <unordered_map>

/* Protocol status codes for FTP server */

/* AUTH */
#define AUTH_REQUEST        "AUTH"
#define STATUS_AUTH_FAILED  430
#define STATUS_AUTH_SUCCESS 230

/* FILES */
#define LIST_REQUEST             "LIST"
#define GET_REQUEST              "GET"
#define STATUS_MISSING_FILE_NAME 400
#define STATUS_LISTING_FILES     150
#define STATUS_FILE_TRANSFER     160
#define STATUS_TRANSFER_COMPLETE 226

/* CHECKSUM */
#define STATUS_CHECKSUM_VERIFY 151

/* ERROR HANDLING */
#define STATUS_FILE_NOT_FOUND  550
#define STATUS_UNKNOWN_COMMAND 500

/* TERMINATION */
#define QUIT_REQUEST   "QUIT"
#define STATUS_GOODBYE 221

#define PORT          12345
#define BUFFER_SIZE   1024
#define MAX_ATTEMPTS  3
#define ATTEMPT_DELAY 2000 // in ms

#define SERVER_CERT "server_cert.pem"
#define SERVER_KEY  "server_key.pem"

/* Loopback addr just to test client and server on same system, ideally you should give the IP addr
 * of the server */
#define SERVER_IP "127.0.0.1"
#define CA_CERT   "server_cert.pem" // Certificate Authority certificate (for the client)

const char* get_error_message(int status_code)
{
  switch (status_code)
  {
    case STATUS_AUTH_FAILED:
      return "--> 430: Too many failed attempts\n";
    case STATUS_AUTH_SUCCESS:
      return "--> 230: Authentication successful!";
    case STATUS_FILE_NOT_FOUND:
      return "--> 550: File not found or invalid file path\n";
    case STATUS_UNKNOWN_COMMAND:
      return "--> 500: Unknown command\n";
    case STATUS_MISSING_FILE_NAME:
      return "--> 400: Missing file name\n";
    case STATUS_LISTING_FILES:
      return "--> 150: Listing files:\n";
    case STATUS_TRANSFER_COMPLETE:
      return "--> 226: Transfer complete\n";
    case STATUS_GOODBYE:
      return "--> 221: Goodbye\n";
    case STATUS_FILE_TRANSFER:
      return "--> 160: Starting File Transfer:\n";
    case STATUS_CHECKSUM_VERIFY:
      return "--> 151: Verifying checksum..\n";
    default:
      return "--> 500: Unknown status code\n"; // Default error for unknown status codes
  }
}

void send_protocol_message(SSL* ssl, int status_code)
{
  const char* error_msg = get_error_message(status_code);
  SSL_write(ssl, error_msg, strlen(error_msg));
}

#endif
