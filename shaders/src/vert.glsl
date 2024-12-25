#version 460
#pragma shader_stage(vertex)

layout(location=0) in vec2 pos_in;

layout(push_constant, std430) uniform pc {
    vec2 position;
    vec2 scale;
    vec4 color;
};

void main() {
    gl_Position = vec4(pos_in * scale + position, 0.0, 1.0);
}