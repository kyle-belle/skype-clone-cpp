#undef __WIN32__
#include "window.h"
#include <GL/glew.h>
#include <GL/glext.h>
#include <GL/wglew.h>
#include <string.h>

#define CREATE_WINDOW_ERROR -2
#define REGISTER_WINDOW_ERROR -1
#define SET_PIXEL_FORMAT_ERROR -3

#define WM_PEEK_SIZE 4545

extern bool running;

LRESULT CALLBACK window_procedue(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam){
    GL_window* gl_window;
    switch(msg){
        case WM_CREATE:{
            CREATESTRUCT* ct = (CREATESTRUCT*)lparam;
            gl_window = (GL_window*)(ct->lpCreateParams);
            SetWindowLongPtr(handle, GWLP_USERDATA, (long)gl_window);
        }
        break;

        case WM_DESTROY:
            running = false;
            PostQuitMessage(0);
        break;

        case WM_SIZE:{
            gl_window = (GL_window*)GetWindowLongPtr(handle, GWLP_USERDATA);

//            printf("size change\n");
            RECT window_size = {0};
            GetClientRect(handle, &window_size)?1:0;

            gl_window->on_change_size((window_size.right - window_size.left), (window_size.bottom - window_size.top));
        }
        break;

        case WM_QUIT:
            running = false;
            PostQuitMessage(0);
        break;

        default:
            return DefWindowProc(handle, msg, wparam, lparam);
    }

    return 0;
}

extern HINSTANCE instance;


GL_window::GL_window(int gl_version_major, int gl_version_minor, const char* wnd_class_name): gl_version_major(gl_version_major), gl_version_minor(gl_version_minor){

    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;

    window_class.cbSize = sizeof(WNDCLASSEX);
    window_class.style = CS_DBLCLKS | CS_OWNDC;
    window_class.lpfnWndProc = window_procedue;
    window_class.hInstance = instance;
    window_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    window_class.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
    window_class.lpszMenuName = NULL;
    window_class.lpszClassName = wnd_class_name;

    if(!RegisterClassEx(&window_class)){
        printf("window register error\n");
    }
}

int GL_window::create_window(int width, int height, const char* window_name){
    window = CreateWindowEx(0, window_class.lpszClassName, "Skyle", WS_VISIBLE | WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, instance, this);

    this->window_name = new char[strlen(window_name) + 1];

    strcpy(this->window_name, window_name);

    if(!window){
        printf("window creation error\n");
        return CREATE_WINDOW_ERROR;
    }

    device_context = GetDC(window);

    pixel_format = ChoosePixelFormat(device_context, &pfd);

    if(!SetPixelFormat(device_context, pixel_format, &pfd)){
        int err = GetLastError();
        printf("error setting pixel format %d\n", err);
        return SET_PIXEL_FORMAT_ERROR;
    }

    if(gl_version_major && gl_version_minor){
        int gl_attribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, gl_version_major,
            WGL_CONTEXT_MINOR_VERSION_ARB, gl_version_minor,
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };

        /// create temporary render_context to access more advanced opengl function
        HGLRC temp_rc = wglCreateContext(device_context);

        wglMakeCurrent(device_context, temp_rc); /// set render context to the windows device context

        /// grabbing opengl function manually
        PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC) wglGetProcAddress("wglCreateContextAttribsARB");

        /// set render context to the windows device context using more advanced version with attributes
        render_context = wglCreateContextAttribsARB(device_context, NULL, gl_attribs);

        wglMakeCurrent(NULL, NULL); /// un-setting the device and render context

        wglDeleteContext(temp_rc); /// delete temp render context

        wglMakeCurrent(device_context, render_context); /// we bind the device and render we actually want
    }else{
        render_context = wglCreateContext(device_context);
        wglMakeCurrent(device_context, render_context);
    }

    return 0;
}

void GL_window::process_window_events(){
    if(PeekMessage(&msg, window, 0, 0, PM_REMOVE)){ /// get window messages

        TranslateMessage(&msg); /// translate massage
        DispatchMessage(&msg); ///dispatch window messages to window procedure
    }
}

void GL_window::on_change_size(int width, int height){
    this->width = width;
    this->height = height;
}

void GL_window::clear_window(){
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.5f, 0.75f, 1.0f, 1.0f);
}

void GL_window::draw_frame(int frame_width, int frame_height, unsigned int size){
    frame_ratio = (float)frame_width/(float)frame_height;
    screen_ratio = (float)width/(float)height;

    vw = (screen_ratio > frame_ratio) ? height * frame_ratio : width;
    vh = (screen_ratio < frame_ratio) ? width / frame_ratio : height;

    glViewport((width - vw)/2, (height - vh)/2, vw, vh);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, size);
}

void GL_window::swap_buffers(){
    SwapBuffers(device_context);
}

