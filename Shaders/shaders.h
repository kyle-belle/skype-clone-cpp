#ifndef SHADERS_H_INCLUDED
#define SHADERS_H_INCLUDED


static const char* vertex_shader_src =
R"a(
    #version 330
    layout (location = 0) in vec2 vertex;
    layout (location = 1) in vec2 tex_coord;

    out vec2 vertex_texture;

    void main(){
        vertex_texture = tex_coord;
        gl_Position = vec4(vertex, 1, 1);
    }
)a";

static const char* fragment_shader_src =
R"a(
    #version 330
    out vec4 color;

    in vec2 vertex_texture;

    uniform sampler2D texel;

    void main(){
        color = texture(texel, vertex_texture);
    }
)a";


#endif // SHADERS_H_INCLUDED
