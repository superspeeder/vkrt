#version 460
#pragma shader_stage(fragment)

layout(location = 0) out vec4 colorOut;

layout(push_constant, std430) uniform pc {
    vec2 position;
    vec2 scale;
    vec4 color;
};

void main() {
    colorOut = color;
}