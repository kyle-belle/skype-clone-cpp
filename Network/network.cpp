#include "network.h"
#include <thread>
#include <stdio.h>

#define IPV4 AF_INET

Network::Network(const char* ip, int port){
    WSADATA wsa_data;
    WORD dllversion = MAKEWORD(2,2);

    if(WSAStartup(dllversion, &wsa_data)){
        printf("failed to start winsock!!!\n");
    }

    socket_address.sin_addr.S_un.S_addr = inet_addr(ip);
    socket_address.sin_port = htons(port);
    socket_address.sin_family = IPV4;

    addr_size = sizeof(socket_address);

    connection = socket(IPV4, SOCK_STREAM, IPPROTO_TCP);
}

void Network::init(const char* ip, int port){
    WSADATA wsa_data;
    WORD dllversion = MAKEWORD(2,2);

    if(WSAStartup(dllversion, &wsa_data)){
        printf("failed to start winsock!!!\n");
    }

    socket_address.sin_addr.S_un.S_addr = inet_addr(ip);
    socket_address.sin_port = htons(port);
    socket_address.sin_family = IPV4;

    addr_size = sizeof(socket_address);

    connection = socket(IPV4, SOCK_STREAM, IPPROTO_TCP);
}

bool Network::connect(){
    if(::connect(connection, (SOCKADDR*)&socket_address, addr_size)){ // returns 0 on success
        // error
        printf("failed to connect to server");
        return false;
    }else{
        MESSAGE_TYPE msg_type;
        get_message_type(msg_type);

        get_message();

        std::thread(input_thread, this, message).detach();
        std::thread(network_thread, this).detach();
        return true;
    }
}


SOCKET& Network::get_connection(){
    return connection;
}


bool Network::get_int(int& data_size){
    int check = 0;

    do{
        int received = recv(connection, ((char*)&data_size) + check, sizeof(int) - check, 0);

        if(received == SOCKET_ERROR){
            printf("failed to get size\n");
            return false;
        }

        check += received;
    }while(check < sizeof(int));



    return true;
}

#define get_size get_int

bool Network::get_data(){
    int data_size;
    int check = 0;

    if(!get_size(data_size)){
        printf("failed to get size\n");
        return false;
    }

    message[data_size] = '\0';

    do{
        int received = recv(connection, message + check, data_size - check, 0);

        if(received == SOCKET_ERROR){
            printf("failed to get message\n");
            printf("%d", WSAGetLastError());
            return false;
        }
        check += received;
    }while(check < data_size);



    if(onReceiveMessage){
        onReceiveMessage(message, data_size);
    }

    return true;
}

#define get_message get_data

bool Network::get_frame(){
    int data_size;
    int check = 0;

    if(!get_size(data_size)){
        printf("failed to get size\n");
        return false;
    }

    frame = new unsigned char[data_size];

    do{
        int received = recv(connection, ((char*)frame) + check, data_size - check, 0);

        if(received == SOCKET_ERROR){
            printf("failed to get message\n");
            printf("%d", WSAGetLastError());
            return false;
        }
        check += received;
    }while(check < data_size);

    if(onReceiveFrame){
        onReceiveFrame(frame, res, data_size);
    }

    return true;
}

bool Network::get_camera_frame(){
    int data_size;
    int check = 0;

    if(!get_size(data_size)){
        printf("failed to get size\n");
        return false;
    }

    frame = new unsigned char[data_size];

    do{
        int received = recv(connection, ((char*)frame) + check, data_size - check, 0);

        if(received == SOCKET_ERROR){
            printf("failed to get message\n");
            printf("%d", WSAGetLastError());
            return false;
        }
        check += received;
    }while(check < data_size);

    if(onReceiveCamFrame){
        onReceiveCamFrame(frame, res, data_size);
    }

    return true;
}


bool Network::get_message_type(MESSAGE_TYPE& msg_type){
    int check = 0;
    do{
        int received = recv(connection, ((char*)&msg_type) + check, sizeof(msg_type) - check, 0);

        if(received == SOCKET_ERROR){
            printf("failed to receive message type\n");
            printf("%d", WSAGetLastError());
            return false;
        }
        check += received;
    }while(check < sizeof(MESSAGE_TYPE));



    return true;
}

bool Network::get_frame_resolution(){
    int check = 0;

    do{
        int received = recv(connection, ((char*)&res) + check, sizeof(Resolution) - check, 0);

        if(received == SOCKET_ERROR){
            printf("failed to receive resolution\n");
            printf("%d", WSAGetLastError());
            return false;
        }
        check += received;
    }while(check < sizeof(Resolution));


    return true;
}

bool Network::send_int(int data_size){
    int check = send(connection, (char*)&data_size, sizeof(int), 0);

    if(check == SOCKET_ERROR){
        printf("failed to send msg type\n");
        return false;
    }

    return true;
}

#define send_size send_int

bool Network::send_message_type(MESSAGE_TYPE msg_type){
    int check = send(connection, (char*)&msg_type, sizeof(msg_type), 0);

    if(check == SOCKET_ERROR){
        printf("failed to send msg type\n");
        return false;
    }

    return true;
}

bool Network::send_frame_resolution(Resolution& res){
    int check = send(connection, (char*)&res, sizeof(Resolution), 0);

    if(check == SOCKET_ERROR){
        printf("failed to send resolution\n");
        printf("%d", WSAGetLastError());
        return false;
    }

    return true;
}

bool Network::send_data(const char* message, MESSAGE_TYPE msg_type){
    unsigned int data_size = strlen(message);

    if(!send_message_type(msg_type)){
        return false;
    }

    if(!send_size(data_size)){
        return false;
    }

    if(send(connection, message, data_size, 0) == SOCKET_ERROR){
        printf("failed to send data\n");
        return false;
    }

    return true;
}

bool Network::send_frame(const unsigned char* frame, MESSAGE_TYPE msg_type, Resolution& res){
    unsigned int data_size = (res.width * res.height) * 4;

    if(!send_message_type(msg_type)){
        return false;
    }

    if(!send_frame_resolution(res)){
        return false;
    }

    if(!send_size(data_size)){
        return false;
    }

    if(send(connection, (char*)frame, data_size, 0) == SOCKET_ERROR){
        printf("failed to send data\n");
        printf("%d", WSAGetLastError());
        return false;
    }

    return true;
}

bool Network::send_camera_frame(const unsigned char* frame, MESSAGE_TYPE msg_type, Resolution& res,unsigned int data_size){
//    unsigned int data_size = (res.width * res.height) * 4;

    if(!send_message_type(msg_type)){
        return false;
    }

    if(!send_frame_resolution(res)){
        return false;
    }

    if(!send_size(data_size)){
        return false;
    }

    if(send(connection, (char*)frame, data_size, 0) == SOCKET_ERROR){
        printf("failed to send data\n");
        printf("%d", WSAGetLastError());
        return false;
    }

    return true;
}

#define send_message send_data

bool Network::process_message(MESSAGE_TYPE& msg_type){

    switch(msg_type){
        case MESSAGE_TYPE::STRING_MESSAGE:
            if(!get_message()){
                return false;
            }

            printf("someone said: %s\n", message);
        break;

        case MESSAGE_TYPE::FRAME_MESSAGE:
            if(!get_frame_resolution()){
                return false;
            }

            if(!get_frame()){
                return false;
            }

        break;

        default:
            printf("unknown message type\n");
    }

    return true;
}

void Network::input_thread(char* message){
    while(true){
        scanf("%255[^\n]s", message);
        fflush(stdin);

        if(!send_message(message, MESSAGE_TYPE::STRING_MESSAGE)){
            printf("error sending message\n");
        }
    }

}

void Network::network_thread(){
    MESSAGE_TYPE message_type;

    while(true){
        if(!get_message_type(message_type)){
            printf("failed to get message type!!\n");
            break;
        };


        if(!process_message(message_type)){
            printf("failed to process message\n");
            break;
        };
    }

    closesocket(connection);
}
