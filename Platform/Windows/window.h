#ifndef PLATFORM_WINDOW_H_INCLUDED
#define PLATFORM_WINDOW_H_INCLUDED

#include <stdio.h>
#include <windows.h>


class GL_window{
    public:

        int pixel_format;
        int gl_version_major;
        int gl_version_minor;
        char* window_name;
        int width, height;
        float frame_ratio;
        float screen_ratio;
        int vw, vh;
        HWND window;
        HDC device_context;
        HGLRC render_context;
        PIXELFORMATDESCRIPTOR pfd;
        WNDCLASSEX window_class = {0};
        MSG msg = {0};

        GL_window(int gl_version_major = 0, int gl_version_minor = 0, const char* wnd_class_name = "window_class");
        ~GL_window() = default;

        int create_window(int width = 640, int height = 480, const char* window_name = "window");
        void clear_window();
        void swap_buffers();
        void process_window_events();
        void on_change_size(int width, int height);

        void draw_frame(int frame_width, int frame_height, unsigned int size);

        static void change_size(int width, int height);
};


#endif // PLATFORM_WINDOW_H_INCLUDED

#ifdef __WIN32__

#define main() WINAPI WinMain(HINSTANCE instance_S, HINSTANCE prev_instance, LPSTR arg, int cmd_line)

HINSTANCE instance;

#define gl_window_init() instance = instance_S; printf("gl_window_init()\n");

#endif // __WIN32__
