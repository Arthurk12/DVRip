#include "dvrip.h"

DVRip::DVRip(std::string t_ip, unsigned short int t_port=34567,  std::string t_username="admin", std::string t_password=""):
m_ip(t_ip),
m_port(t_port),
m_username(t_username),
m_hashedPassword(t_password/*suposed to hash password before assign*/){
    
    m_socketAddress.sin_family = AF_INET;
    m_socketAddress.sin_port = htons(t_port);
    m_socketAddress.sin_addr.s_addr = inet_addr(t_ip.c_str());
    memset(m_socketAddress.sin_zero, '\0', sizeof(m_socketAddress.sin_zero));
}

DVRip::~DVRip(){

}

void DVRip::connectSocket(){
    
    m_socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);

    if(m_socketFileDescriptor < 0){
        std::cerr << "Error: could not create socket!";
        return;
    }

    if(connect(m_socketFileDescriptor, (const struct sockaddr *)&m_socketAddress, sizeof(m_socketAddress))<0){
        std::cerr << "Error: could not connect to given ip/port!";
        return;
    }

}

void DVRip::send(unsigned short int t_msgCode, json t_data={}){
    
    std::stringstream packet;
    char packetEnd[2] = {0x0a, 0x00};
    char bytePad[2] = {0x00, 0x00};

    unsigned short int magicValue = 255;
    unsigned int zero = 0;
    unsigned int dataLength = t_data.dump().size() + sizeof(packetEnd);

    packet.write(reinterpret_cast<const char*>(&magicValue), sizeof(char));             //1byte
    packet.write(reinterpret_cast<const char*>(&zero), sizeof(char));                   //1byte
    packet.write(reinterpret_cast<const char*>(&bytePad[0]), sizeof(bytePad));          //2bytes
    packet.write(reinterpret_cast<const char*>(&m_session), sizeof(m_session));         //4bytes
    packet.write(reinterpret_cast<const char*>(&m_packetCount), sizeof(m_packetCount)); //4bytes
    packet.write(reinterpret_cast<const char*>(&bytePad[0]), sizeof(bytePad));          //2bytes
    packet.write(reinterpret_cast<const char*>(&t_msgCode), sizeof(t_msgCode));         //2bytes
    packet.write(reinterpret_cast<const char*>(&dataLength), sizeof(dataLength));       //4bytes
    packet.write(reinterpret_cast<const char*>(&t_data.dump()[0]), t_data.dump().size());
    packet.write(reinterpret_cast<const char*>(&packetEnd[0]), sizeof(packetEnd));

    write(m_socketFileDescriptor, packet.str().c_str(), packet.str().size());

    char header[20];
    int bytesRead=0;

    bytesRead = read(m_socketFileDescriptor, &header, sizeof(header));
    if(bytesRead==-1){
        std::cerr << "Error: could not read server reply header!";
        return;
    }else{
        unsigned int &lenData = reinterpret_cast<unsigned int&>(header[16]);
        std::cout << lenData;
        char replyData[lenData];
        int dataBytesRead = 0;

        dataBytesRead = read(m_socketFileDescriptor, &replyData, sizeof(replyData));
        std::cout << json::parse((const char*)replyData);
    }

}

void DVRip::login(){
    connectSocket();
    
    json packet;
    packet["EncryptType"] = "MD5";
    packet["LoginType"] = "DVRIP-Web";
    packet["PassWord"] = m_hashedPassword;
    packet["UserName"] = m_username;
    
    send(1000, packet);
}