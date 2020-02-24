#include "opengl_functions.h"
#include <stdio.h>
#include <GL/glew.h>

void init_glew(){
    glewExperimental = true;

    if(glewInit() != GLEW_OK){
        printf("glew initialization error\n");
    }
}

void init_opengl(){
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void enable(int gl_enum){
    glEnable(gl_enum);
}

void create_vertex_array(unsigned int& vao){
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
}

void create_vertex_buffer(unsigned int& vbo, const float* vertices, unsigned int size){
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void create_texture_coordinates(unsigned int& tbo, const float* tex_coords, unsigned int size){
    glGenBuffers(1, &tbo);
    glBindBuffer(GL_ARRAY_BUFFER, tbo);
    glBufferData(GL_ARRAY_BUFFER, size, tex_coords, GL_STATIC_DRAW);

    glVertexAttribPointer(1, 2, GL_FLOAT, false, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
}

void create_texture(unsigned int& tex, unsigned char* image, int width, int height){
    glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    float border_color[4] = {0,0,0,0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

}

void update_texture(unsigned int& tex, unsigned char* image,int width, int height){

    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
}

void create_shaders(unsigned int& shader_program, const char* vertex_shader_src, const char* fragment_shader_src){
    unsigned int vertex_shader, fragment_shader;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_src, NULL);
    glCompileShader(vertex_shader);

    int status;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status);

    if(!status){
        int size;
        glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &size);
        char* shader_log = new char[size];

            glGetShaderInfoLog(vertex_shader, size, &size, shader_log);

            printf("%s\n", shader_log);

        delete[] shader_log;
    }

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_src, NULL);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &status);
    if(!status){
        int size;
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &size);
        char* shader_log = new char[size];

            glGetShaderInfoLog(fragment_shader, size, &size, shader_log);

            printf("%s\n", shader_log);

        delete[] shader_log;
    }

    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &status);
    if(!status){
        int size;
        glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &size);
        char* program_log = new char[size];

            glGetProgramInfoLog(fragment_shader, size, &size, program_log);

            printf("%s\n", program_log);

        delete[] program_log;
    }


    glValidateProgram(shader_program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    glUseProgram(shader_program);

}

void upload_texture_uniform(unsigned int& shader_program){
    glUniform1i(glGetUniformLocation(shader_program, "texel"), 0);
}
