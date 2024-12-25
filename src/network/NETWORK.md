# NETWORK

Working on a custom implementation of FTP server

## Current implementation

Just a basic challenge security auth using openssl and some back and forth signal receiving in a traditional CS arch

-> Allows for one file download from server (directory is hardcoded atm)

## SSL security

We are using openssl for a more safer socket connection and transmission

To generate server key and certificate:

```bash
# Give the Common name as your server's IP addr, email is not necessary
openssl req -x509 -nodes -days 365 -newkey rsa:2048 -keyout server_key.pem -out server_cert.pem
```

Make sure that the server certificate is sent to your other device (and the client code obviously)

> [!NOTE]
> 
> You will need to have termux installed and configured in order to compile client code in your phone
> 
> IOS users sadly cannot use this feature
> 
> **MAKE SURE THAT YOU SETUP TERMUX IN YOUR PHONE BY RUNNING `termux-setup-storage`**
> 
> This is to get access to the Internal Storage of your android phone in termux itself
> 
> ```bash
> # To install compiler and compiling client.cpp in your phone (android only)
> pkg install clang libllvm
> clang++ client.cpp -lcrypto -lssl -o client
> ./client # make sure you have server_cert.pem in your phone already
> ```
> 

## Goal

The goal of this server is to easily transfer your local songs directory from your PC to phone without a hassle

