#include <stdio.h>
#include <winsock.h>
#include <thread>

#define LOCALHOST "127.0.0.1"
#define ANYHOST "0.0.0.0"
#define IPV4 AF_INET
#define MAX_SOCKETS 5
#define PORT 2222

enum class MESSAGE_TYPE{
    STRING_MESSAGE = 0, FRAME_MESSAGE
};

int connection_counter = 0;

SOCKET connections[MAX_SOCKETS];

struct Resolution{
   int width, height;
};

bool get_int(int ID, unsigned int& data_size){
    int check = 0;

    do{
        int received = recv(connections[ID], ((char*)&data_size)+check, sizeof(int) - check, 0);

        if(received == SOCKET_ERROR){
            printf("failed to get size\n");
            printf("%d", WSAGetLastError());
            return false;
        }

        check += received;
    }while(check < sizeof(int));

    return true;
}

#define get_size get_int

bool get_data(int ID, char*& message){
    unsigned int data_size;
    int check = 0;

    if(!get_size(ID, data_size)){
        printf("failed to get size\n");
        return false;
    }

    message = new char[data_size + 1];
    message[data_size] = '\0';

    do{
        int received = recv(connections[ID], message + check, data_size - check, 0);

        if(received == SOCKET_ERROR){
            printf("failed to get message\n");
            printf("%d", WSAGetLastError());
            return false;
        }
        check += received;
    }while(check < data_size);

    return true;
}

#define get_message get_data

bool get_frame(int ID, unsigned char*& frame, unsigned int data_size){
    int check = 0;
    frame = new unsigned char[data_size];

    do{
        int received = recv(connections[ID], ((char*)frame) + check, data_size - check, 0);

        if(received == SOCKET_ERROR){
            printf("failed to get message\n");
            printf("%d", WSAGetLastError());
            return false;
        }

        check += received;
    }while(check < data_size);

    return true;
}

bool get_message_type(int ID, MESSAGE_TYPE& msg_type){
    int check = 0;

    do{
        int received = recv(connections[ID], ((char*)&msg_type) + check, sizeof(msg_type) - check, 0);

        if(received == SOCKET_ERROR){
            printf("failed to receive message type\n");
            printf("%d", WSAGetLastError());
            return false;
        }

        check += received;
    }while(check < sizeof(MESSAGE_TYPE));

    return true;
}


bool get_frame_resolution(int ID, Resolution& res){
    int check = 0;

    do{
        int received = recv(connections[ID], ((char*)&res) + check, sizeof(Resolution) - check, 0);

        if(received == SOCKET_ERROR){
            printf("failed to receive resolution\n");
            printf("%d", WSAGetLastError());
            return false;
        }

        check += received;
    }while(check < sizeof(Resolution));

    return true;
}

bool send_int(int ID, int data_size){
    int check = send(connections[ID], (char*)&data_size, sizeof(int), 0);

    if(check == SOCKET_ERROR){
        printf("failed to send msg type\n");
        printf("%d", WSAGetLastError());
        return false;
    }

    return true;
}

#define send_size send_int


bool send_message_type(int ID, MESSAGE_TYPE msg_type){
    int check = send(connections[ID], (char*)&msg_type, sizeof(msg_type), 0);

    if(check == SOCKET_ERROR){
        printf("failed to send msg type\n");
        printf("%d", WSAGetLastError());
        return false;
    }

    return true;
}

bool send_frame_resolution(int ID, Resolution& res){
    int check = send(connections[ID], (char*)&res, sizeof(Resolution), 0);

    if(check == SOCKET_ERROR){
        printf("failed to send resolution\n");
        printf("%d", WSAGetLastError());
        return false;
    }

    return true;
}

bool send_data(int ID, const char* message, MESSAGE_TYPE msg_type){
    unsigned int data_size = strlen(message);

    if(!send_message_type(ID, msg_type)){
        return false;
    }

    if(!send_size(ID, data_size)){
        return false;
    }

    if(send(connections[ID], message, data_size, 0) == SOCKET_ERROR){
        printf("failed to send data\n");
        printf("%d", WSAGetLastError());
        return false;
    }

    return true;

}

#define send_message send_data

bool send_frame(int ID, unsigned char* frame, MESSAGE_TYPE msg_type, Resolution& res, unsigned int data_size){
//    unsigned int data_size = (res.width * res.height) * 4;

    if(!send_message_type(ID, msg_type)){
        return false;
    }

    if(!send_frame_resolution(ID, res)){
        return false;
    }

    if(!send_size(ID, data_size)){
        return false;
    }

    if(send(connections[ID], (char*)frame, data_size, 0) == SOCKET_ERROR){
        printf("failed to send frame\n");
        printf("%d", WSAGetLastError());
        return false;
    }

    return true;
}

bool process_message(int ID, MESSAGE_TYPE& msg_type){
    char* message;
    unsigned char* frame = nullptr;
    Resolution res;
    unsigned int data_size;

    switch(msg_type){
        case MESSAGE_TYPE::STRING_MESSAGE:
            if(!get_message(ID, message)){
                return false;
            }


            for(int i = 0; i < connection_counter; i++){
                if(i == ID){
                    continue;
                }

                if(!send_message(i, message, msg_type)){
                    return false;
                }

                printf("MESSAGE SENT\n");
            }
        break;

        case MESSAGE_TYPE::FRAME_MESSAGE:

            if(!get_frame_resolution(ID, res)){
                return false;
            }


            if(!get_size(ID, data_size)){
                printf("failed to get size\n");
                return false;
            }

            if(!get_frame(ID, frame, data_size)){
                return false;
            }


            for(int i = 0; i < connection_counter; i++){
                if(i == ID){
                    continue;
                }

                if(!send_frame(i, frame, msg_type, res, data_size)){
                    return false;
                }

                printf("FRAME SENT from ID: %d to ID: %d\n", ID, i);
            }

            if(frame){
                delete[] frame;
            }
        break;

        default:
            printf("unknown message type: %d\n", msg_type);
    }

    return true;
}


void client_handler(int ID){
    MESSAGE_TYPE message_type;

    while(true){
        if(!get_message_type(ID, message_type)){
            printf("failed to get message type!!\n");
            break;
        };


        if(!process_message(ID, message_type)){
            printf("failed to process message\n");
            break;
        };
    }

    closesocket(connections[ID]);
}

int main()
{
    printf("hello world\n");

    WSADATA wsa_data;
    WORD dllversion = MAKEWORD(2,2);

    if(WSAStartup(dllversion, &wsa_data)){
        printf("failed to start winsock!!!\n");
    }

    SOCKADDR_IN socket_bind_address = {0};
    socket_bind_address.sin_addr.S_un.S_addr = inet_addr(ANYHOST);
    socket_bind_address.sin_port = htons(PORT);
    socket_bind_address.sin_family = IPV4;


    int addr_size = sizeof(socket_bind_address);

    SOCKET listener_socket = socket(IPV4, SOCK_STREAM, IPPROTO_TCP);
    bind(listener_socket, (SOCKADDR*)&socket_bind_address, addr_size);
    listen(listener_socket, SOMAXCONN);

    SOCKET new_connection = {0};

    for(int i = 0; i < MAX_SOCKETS; i++){
        new_connection = accept(listener_socket, (SOCKADDR*)&socket_bind_address, &addr_size);

        if(new_connection){
            printf("CLIENT CONNECTED\n");
            connections[i] = new_connection;

            const char* message = "HEY FROM SERVER!!!\n";
            send_message(i, message, MESSAGE_TYPE::STRING_MESSAGE);

            connection_counter++;

            std::thread(client_handler, i).detach();
        }else{
            printf("new connection failed\n");
        }
    }

    while(true){

    }
}
