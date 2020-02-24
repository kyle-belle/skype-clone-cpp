#ifndef NETWORK_H_INCLUDED
#define NETWORK_H_INCLUDED

#include <winsock.h>

enum class MESSAGE_TYPE{
    STRING_MESSAGE = 0, FRAME_MESSAGE
};

struct Resolution{
   int width, height;
};

class Network{
public:

    bool can_send_frame = true;
    int addr_size;
    SOCKADDR_IN socket_address;
    SOCKET connection;
    char message[256];
    char input_message[256];
    Resolution res;
    Resolution frame_res;
    unsigned char* frame;
                            // message  //size
    void (*onReceiveMessage)(const char*, int);

    void (*onReceiveFrame) (unsigned char*, Resolution&, unsigned int);

    void (*onReceiveCamFrame) (unsigned char*, Resolution&, unsigned int);


    Network(){}

    Network(const char* ip, int port);
    void init(const char* ip, int port);
    bool connect();

    SOCKET& get_connection();

    bool get_int(int& data_size);
    #define get_size get_int

    bool get_data();

    #define get_message get_data

    bool get_frame();

    bool get_camera_frame();

    bool get_message_type(MESSAGE_TYPE& msg_type);

    bool get_frame_resolution();

    bool send_int(int data_size);

    #define send_size send_int

    bool send_message_type(MESSAGE_TYPE msg_type);

    bool send_frame_resolution(Resolution& res);

    bool send_data(const char* message, MESSAGE_TYPE msg_type);

    bool send_frame(const unsigned char* frame, MESSAGE_TYPE msg_type, Resolution& res);

    bool send_camera_frame(const unsigned char* frame, MESSAGE_TYPE msg_type, Resolution& res, unsigned int data_size);
    #define send_message send_data

    bool process_message(MESSAGE_TYPE& msg_type);

    void input_thread(char* message);

    void network_thread();
};

#endif // NETWORK_H_INCLUDED
