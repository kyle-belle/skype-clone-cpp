#ifndef OPENGL_FUNCTIONS_H_INCLUDED
#define OPENGL_FUNCTIONS_H_INCLUDED

void init_glew();

void init_opengl();

void enable(int gl_enum);

void create_vertex_array(unsigned int& vao);

void create_vertex_buffer(unsigned int& vbo, const float* vertices, unsigned int size);

void create_texture_coordinates(unsigned int& tbo, const float* tex_coords, unsigned int size);

void create_texture(unsigned int& tex, unsigned char* image,int width, int heigth);

void update_texture(unsigned int& tex, unsigned char* image,int width, int height);

void create_shaders(unsigned int& shader_program, const char* vertex_shader_src, const char* fragment_shader_src);

void upload_texture_uniform(unsigned int& shader_program);

#endif // OPENGL_FUNCTIONS_H_INCLUDED
