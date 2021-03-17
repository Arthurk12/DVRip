#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <iostream>
#include <sstream>
#include <openssl/md5.h>
#include "include/json.hpp"

#define sofiaHashChars "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"


using json = nlohmann::json;

class DVRip{

//functions
public:
    DVRip(std::string t_ip, unsigned short int t_port,  std::string t_username, std::string t_password);
    ~DVRip();
    bool login();

private:
    char* sofiaHash(std::string t_rawPassword);
    void connectSocket();
    json send(unsigned short int t_msgCode, json t_data={}, bool t_waitResponse=true);
    json receiveJson();



//variables
public:
    

private:
    std::string m_ip;
    unsigned short int m_port;
    struct sockaddr_in m_socketAddress;
    std::string m_username;
    std::string m_hashedPassword;
    int m_socketFileDescriptor  =0;

    std::string m_session;
    unsigned int m_packetCount  =0;

    const int OK_CODES[2] = {100, 515};

};