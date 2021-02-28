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
#include "include/json.hpp"


using json = nlohmann::json;

class DVRip{

//functions
public:
    DVRip(std::string t_ip, unsigned short int t_port,  std::string t_username, std::string t_password);
    ~DVRip();
    void login();

private:
    void connectSocket();
    void send(unsigned short int t_msgCode, json t_data);
    void receive();


//variables
public:
    

private:
    std::string m_ip;
    unsigned short int m_port;
    struct sockaddr_in m_socketAddress;
    std::string m_username;
    std::string m_hashedPassword;
    int m_socketFileDescriptor  =0;

    unsigned int m_session      =0;
    unsigned int m_packetCount  =0;

};