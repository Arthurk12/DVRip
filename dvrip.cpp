#include "dvrip.h"

DVRip::DVRip(std::string t_ip, unsigned short int t_port=34567,  std::string t_username="admin", std::string t_password=""):
m_ip(t_ip),
m_port(t_port),
m_username(t_username),
m_hashedPassword(sofiaHash(t_password)){
    
    m_socketAddress.sin_family = AF_INET;
    m_socketAddress.sin_port = htons(t_port);
    m_socketAddress.sin_addr.s_addr = inet_addr(t_ip.c_str());
    memset(m_socketAddress.sin_zero, '\0', sizeof(m_socketAddress.sin_zero));
}

DVRip::~DVRip(){

}

char* DVRip::sofiaHash(std::string t_rawPassword){
    unsigned char hashDigest[MD5_DIGEST_LENGTH];
    MD5((const unsigned char*) t_rawPassword.c_str(), t_rawPassword.length(), hashDigest);

    char *hashedPassword;

    for(int i = 0; i < MD5_DIGEST_LENGTH; i+=2){
        hashedPassword[i/2] = sofiaHashChars[(hashDigest[i]+hashDigest[i+1])%62];
    }

    return hashedPassword;
}

void DVRip::connectSocket(){
    
    m_socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);


    if(m_socketFileDescriptor < 0){
        std::cerr << "Error: could not create socket!"<< std::endl;
        return;
    }

    if(connect(m_socketFileDescriptor, (const struct sockaddr *)&m_socketAddress, sizeof(m_socketAddress))<0){
        std::cerr << "Error: could not connect to given ip/port!"<< std::endl;
        return;
    }
    

    struct timeval defaultTimeout;
    defaultTimeout.tv_sec = 5;
    defaultTimeout.tv_usec = 0;
    setsockopt(m_socketFileDescriptor, SOL_SOCKET, SO_RCVTIMEO, (const char*)&defaultTimeout, sizeof(defaultTimeout));

}

json DVRip::send(unsigned short int t_msgCode, json t_data, bool t_waitResponse){
    
    std::stringstream packet;
    char packetEnd[2] = {0x0a, 0x00};
    char bytePad[2] = {0x00, 0x00};

    unsigned short int magicValue = 255;
    unsigned int zero = 0;
    unsigned int dataLength = t_data.dump().size() + sizeof(packetEnd);

    packet.write(reinterpret_cast<const char*>(&magicValue),        sizeof(char));              //1byte
    packet.write(reinterpret_cast<const char*>(&zero),              sizeof(char));              //1byte
    packet.write(reinterpret_cast<const char*>(&bytePad[0]),        sizeof(bytePad));           //2bytes
    packet.write(reinterpret_cast<const char*>(&m_session),         4);                         //4bytes
    packet.write(reinterpret_cast<const char*>(&m_packetCount),     sizeof(m_packetCount));     //4bytes
    packet.write(reinterpret_cast<const char*>(&bytePad[0]),        sizeof(bytePad));           //2bytes
    packet.write(reinterpret_cast<const char*>(&t_msgCode),         sizeof(t_msgCode));         //2bytes
    packet.write(reinterpret_cast<const char*>(&dataLength),        sizeof(dataLength));        //4bytes
    packet.write(reinterpret_cast<const char*>(&t_data.dump()[0]),  t_data.dump().size());
    packet.write(reinterpret_cast<const char*>(&packetEnd[0]),      sizeof(packetEnd));

    write(m_socketFileDescriptor, packet.str().c_str(), packet.str().size());

    json reply;

    if(t_waitResponse)
        reply = receiveJson();
        
    return reply;
}

json DVRip::receiveJson(){
    char header[20];
    int bytesRead =0;

    json unknownError;
    unknownError["Ret"] = 101;      //error code for "Unknown error"

    bytesRead = read(m_socketFileDescriptor, &header, sizeof(header));
    
    if(bytesRead>0){
        unsigned int &dataLength = reinterpret_cast<unsigned int&>(header[16]);
        char replyData[dataLength];
        int dataBytesRead = 0;
        
        dataBytesRead = read(m_socketFileDescriptor, &replyData, sizeof(replyData));
        
        if(dataBytesRead>0){
            m_packetCount++;
            return json::parse((const char*) replyData);
        }else{
            std::cerr << "Error: could not read server reply data!"<< std::endl;
            return unknownError;
        }
    }else{
        std::cerr << "Error: could not read server reply header!"<< std::endl;
        return unknownError;
    }
}

bool DVRip::login(){
    connectSocket();
    
    json packet;
    packet["EncryptType"] = "MD5";
    packet["LoginType"] = "DVRIP-Web";
    packet["PassWord"] =  m_hashedPassword;
    packet["UserName"] = m_username;
    
    json reply = send(1000, packet);    //1000 is login message code

    if(std::find(std::begin(OK_CODES), std::end(OK_CODES), reply["Ret"].get<int>()) != std::end(OK_CODES)){
        m_session = reply["SessionID"].get<std::string>();
        std::cerr << reply << std::endl;
        return true;
    }else{
        std::cerr << "Error on login code: " << reply["Ret"] << std::endl;
        return false;
    }
}

