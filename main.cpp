#include "functions/opengl/opengl_functions.h"
#include <GL/glew.h>
#include <STB/stb_image.h>
#include <conio.h>
#include "Shaders/shaders.h"
#include "window.h"
#include <thread>
#include <winsock.h>
#include "Network/network.h"
#include "vfw.h"
#include "math.h"
#include "time.h"

bool running = true;
bool completed = false;
bool can_send_frame = true;

float vertices[] = {
    -1.0f, -1.0f,
     1.0f, -1.0f,
    -1.0f, 1.0f,
    1.0f, 1.0f
};

float tex_coords[] = {
    0.0f, 1.0f,
    1.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f
};

#define LOCALHOST "127.0.0.1"
#define ANYHOST "0.0.0.0"
#define PORT 2222

unsigned char* frame = nullptr;
Resolution res;

Network network;

void onReceiveMessage(const char* received_message, int message_size){
    printf("someone said %s\n", received_message);
}

int clip(int num){
    return (int)fmax(0, fmin(255, num));
}

void onReceiveFrame(unsigned char* received_frame, Resolution& receive_res, unsigned int data_size){

    clock_t start, end;

    start = clock();

    res = receive_res;
    frame = new unsigned char[(receive_res.width * receive_res.height * 4)];

    unsigned char* input_pixels = received_frame;
    unsigned char* output_pixels = frame;

    for(unsigned int i = 0; i < data_size/4; ++i){

        int y0 = input_pixels[0];
        int u0 = input_pixels[1];
        int y1 = input_pixels[2];
        int v0 = input_pixels[3];
        input_pixels += 4;

        int c = y0 - 16;
        int d = u0 - 128;
        int e = v0 - 128;

        output_pixels[3] = 255;
        output_pixels[7] = 255;

        // first pixel
        output_pixels[2] = clip((298 * c + 516 * d + 128) >> 8); // blue
        output_pixels[1] = clip((298 * c - 100 * d - 208 * e + 128) >> 8); //green
        output_pixels[0] = clip((298 * c + 409 * e + 128) >> 8); // red

        // second pixel
        c = y1 - 16;
        output_pixels[6] = clip((298 * c + 516 * d + 128) >> 8); // blue
        output_pixels[5] = clip((298 * c - 100 * d - 208 * e + 128) >> 8); //green
        output_pixels[4] = clip((298 * c + 409 * e + 128) >> 8); // red
        output_pixels += 8;
    }

    completed = true;

    end = clock();

    printf("total time for frame conversion: %.2fsec\n", (double)(end - start)/CLOCKS_PER_SEC);

    delete received_frame;

}

struct Capture_user_data{
    unsigned* TEX;
    unsigned char* frame;
    CAPSTATUS* capture_status;
    Resolution* resolution;
};

void print_capture_status(CAPSTATUS& c){
    printf("CAPTURE STATUS\n-------------------\nis capturing: %s\nwidth: %dpx Height: %dpx\n# OF FRAMES: %lu\nhas capture file: %s\n", c.fCapturingNow?"true":"false", c.uiImageWidth, c.uiImageHeight, c.dwCurrentVideoFrame, c.fCapFileExists?"true":"false");
}

void print_capture_driver_capabilities(CAPDRIVERCAPS& c){
    printf("driver #%d\n----------------------\nhas overlay: %s\nhas DlgVideoSource: %s\nhas DlgVideoFormat: %s\nhas DlgVideoDisplay: %s\nCapture Initialized: %s\nDriver Supplies Palettes: %s\n", c.wDeviceIndex, c.fHasOverlay?"true":"false", c.fHasDlgVideoSource?"true":"false", c.fHasDlgVideoFormat?"true":"false", c.fHasDlgVideoDisplay?"true":"false", c.fCaptureInitialized?"true":"false", c.fDriverSuppliesPalettes?"true":"false");
}

LRESULT PASCAL FrameCallback(HWND hwnd, LPVIDEOHDR lpvid){
    if(!hwnd)
        return false;


    Capture_user_data* cud = (Capture_user_data*)capGetUserData(hwnd);
    capGetStatus(hwnd, cud->capture_status, sizeof(CAPSTATUS));

    cud->resolution->width = cud->capture_status->uiImageWidth;
    cud->resolution->height = cud->capture_status->uiImageHeight;

    if(!network.send_camera_frame(lpvid->lpData, MESSAGE_TYPE::FRAME_MESSAGE, *cud->resolution, lpvid->dwBufferLength)){
        printf("failed to send frame\n");
    }

    return (LRESULT) true;
}

Capture_user_data capture_user_data;

int main()
{
    char driver_name[80];
    char driver_version[80];
    HWND capture_handle;
    CAPSTATUS capture_status;
    CAPDRIVERCAPS driver_capabilities;
    CAPTUREPARMS capture_parameters;

    for(int i = 0; i < 10; i++){
        if(capGetDriverDescription(i, driver_name, sizeof(driver_name), driver_version, sizeof(driver_version))){
            printf("Driver #%d Name: %s Version: %s\n", i, driver_name, driver_version);
        }
    }

    network.onReceiveMessage = onReceiveMessage;
    network.onReceiveFrame = onReceiveFrame;


    network.init(LOCALHOST, PORT);

    if(!network.connect()){
        printf("failed to connect to server\n");
    }

    int width, example_width, height, example_height, channels, example_channels;
    unsigned char* image = stbi_load("./instagram.png", &width, &height, &channels, 4);

    if(!image){
        printf("image loading failed\n");
    }else{
        res.width = width;
        res.height = height;
        frame = image;
    }

    unsigned char* example_image = stbi_load("./bmwM3GTR.jpg", &example_width, &example_height, &example_channels, 4);

    if(!example_image){
        printf("example image loading failed\n");
    }

    printf("hello World!!\n");

    gl_window_init();

    GL_window gl_window(3, 3, "skyle");

    gl_window.create_window(1280, 720, "Skyle");

    //create capture window
    capture_handle = capCreateCaptureWindow("Capture Window", WS_CHILD, 0, 0, 0, 0, gl_window.window, 1);

    //connect to capture driver
    capDriverConnect(capture_handle, 0);

    capSetCallbackOnFrame(capture_handle, FrameCallback);
    capSetCallbackOnVideoStream(capture_handle, FrameCallback);

    //get capture status
    capGetStatus(capture_handle, &capture_status, sizeof(capture_status));
    print_capture_status(capture_status);

//    SetWindowPos(capture_handle, NULL, 0, 0, capture_status.uiImageWidth, capture_status.uiImageHeight, 0);

    //get driver capabilities
    capDriverGetCaps(capture_handle, &driver_capabilities, sizeof(driver_capabilities));
    print_capture_driver_capabilities(driver_capabilities);

    if(driver_capabilities.fHasDlgVideoSource){
        capDlgVideoSource(capture_handle);
        capGetStatus(capture_handle, &capture_status, sizeof(capture_status));
    }

    if(driver_capabilities.fHasDlgVideoFormat){
        capDlgVideoFormat(capture_handle);
        capGetStatus(capture_handle, &capture_status, sizeof(capture_status));
    }

    capPreviewRate(capture_handle, 33);
    capPreview(capture_handle, false);
    float fps = 30.0f;

    capCaptureGetSetup(capture_handle, &capture_parameters, sizeof(CAPTUREPARMS));

    capture_parameters.dwRequestMicroSecPerFrame = (1.0e6 / fps);
    capture_parameters.fYield = true;
    capture_parameters.fAbortLeftMouse = false;
    capture_parameters.fAbortRightMouse = false;

    capCaptureSetSetup(capture_handle, &capture_parameters, sizeof(capture_parameters));

    init_glew();
    init_opengl();

    enable(GL_BLEND);

    printf("opengl version: %s\n", glGetString(GL_VERSION)); /// get the opengl version

    unsigned int VAO, VBO, TBO, TEX, shader_program;
    create_vertex_array(VAO);

    create_shaders(shader_program, vertex_shader_src, fragment_shader_src);

    create_vertex_buffer(VBO, vertices, sizeof(vertices));

    create_texture_coordinates(TBO, tex_coords, sizeof(tex_coords));

    create_texture(TEX, frame, width, height);

    stbi_image_free(image);

    capture_user_data.capture_status = &capture_status;
    capture_user_data.frame = frame;
    capture_user_data.TEX = &TEX;
    capture_user_data.resolution = &res;

    capSetUserData(capture_handle, (LPARAM)&capture_user_data);

    while(running){

        gl_window.process_window_events();

        if(GetAsyncKeyState(VK_SPACE) & 1){
//            Resolution frame_res;
//            frame_res.width = example_width;
//            frame_res.height = example_height;
//
//            if(!network.send_frame(example_image, MESSAGE_TYPE::FRAME_MESSAGE, frame_res)){
//                printf("failed to send frame");
//            }
            if(!capture_status.fCapturingNow)
                capCaptureSequenceNoFile(capture_handle);
        }

        if(completed){
            update_texture(TEX, frame, res.width, res.height);
            delete[] frame;
            completed = false;

            gl_window.clear_window();

            gl_window.draw_frame(res.width, res.height, sizeof(vertices)/(sizeof(float) * 2));

            gl_window.swap_buffers();
        }

    }

    printf("connection to server lost.\n");

    return 0;
}
