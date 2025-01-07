#version 460
#pragma shader_stage(vertex)

layout(location=0) in vec2 pos_in;

layout(set=0, binding=0) uniform ubo {
    mat4 view_projection;
};

layout(push_constant, std430) uniform pc {
    vec2 position;
    vec2 scale;
    vec4 color;
};

void main() {
    gl_Position = view_projection * vec4(pos_in * scale + position, 0.0, 1.0);
}