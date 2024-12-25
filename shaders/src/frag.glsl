#version 460
#pragma shader_stage(fragment)

#include <lib.glsl>

layout(location = 0) out vec4 colorOut;

void main() {
    colorOut = test_color();
}