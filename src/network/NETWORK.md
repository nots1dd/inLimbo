# NETWORK

Working on a custom implementation of FTP server

## Current implementation

Just a basic challenge security auth using openssl and some back and forth signal receiving in a traditional CS arch

-> Allows for one file download from server (directory is hardcoded atm)

## SSL security

We are using openssl for a more safer socket connection and transmission

To generate server key and certificate: (**RUN IN YOUR SERVER(PC) AND SEND IT TO YOUR PHONE**)

```bash
# Give the Common name as your server's IP addr, email is not necessary
openssl req -x509 -nodes -days 365 -newkey rsa:2048 -keyout server_key.pem -out server_cert.pem
```

Make sure that the following files are sent over to your phone using this command:

```bash
# You will have to also send over the servers' certificate:- server_cert.pem file to your phone
mkdir inLimbo_client
cd inLimbo_client/
curl -L -o client.cpp https://raw.githubusercontent.com/nots1dd/inLimbo/main/src/network/client.cpp
curl -L -o protocols.h https://raw.githubusercontent.com/nots1dd/inLimbo/main/src/network/protocols.h
```

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

-> The server will have access to the password hash that you will give in config.toml file

-> The client will input the password and compute the sha256 hash of salt+password and raise a challenge with the server's hash 

-> If they match, we autheticate successfully, else it fails 

-> Needless to say, the client also needs the `server_cert.pem` file as well else client can never connect

The above forms a baseline security and safety of your connection with the FTP server, it may not be the most secure nor the ideal process to go about securing a FTP server, but it works.

## Goal

The goal of this server is to easily transfer your local songs directory from your PC to phone without a hassle

