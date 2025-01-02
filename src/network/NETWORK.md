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
> **CAUTION:** If client refuses to connect and you are sure that everything is done as explained,
> 
> -> Consider changing the `SERVER_IP` macro in `protocols.h` to the server's ip address (you can find it out in linux using `ip a`)
> 
> In code it should look something like this:
> 
> ```cpp 
> #define SERVER_IP "127.0.0.1" // by default it is set to loopback addr, set this to the actual servers ip
> ```
> 
> I will come up with a more dynamic way of setting this in the future but right now it is pretty rigid.
> 
-> The server will have access to the password hash that you will give in config.toml file

-> The client will input the password and compute the sha256 hash of salt+password and raise a challenge with the server's hash 

-> If they match, we autheticate successfully, else it fails 

-> Needless to say, the client also needs the `server_cert.pem` file as well else client can never connect

The above forms a baseline security and safety of your connection with the FTP server, it may not be the most secure nor the ideal process to go about securing a FTP server, but it works.

If it is not obvious enough, the default password is `password`.

> [!NOTE]
> 
> **TO CHANGE YOUR PASSWORD**:
> 
> It is simple enough actually, follow these steps:
> 
> 1. Change salt if you want to, upon choosing give it in your `config.toml` file **AND** your client.cpp code! 
> 2. Get a password of your choosing and concatenate salt and password (salt + password -> in this order)
> 3. Get its SHA256 hash of the concatenated string (can go to an [online alternative](https://emn178.github.io/online-tools/sha256.html)) or just use openssl or other binaries to find its hash 
> 4. Put the hash in `config.toml` under password_hash field
> 
> You are all done!
> 

Currently *SALT* and *USERNAME* are hardcoded and tedious to change, but this will change in the future

## Goal

The goal of this server is to easily transfer your local songs directory from your PC to phone without a hassle

