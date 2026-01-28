#version 460 core
#include <common.sp>

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 color;

layout(location = 0) out vec2 uvs;
layout(location = 1) out vec3 colors;

void main() {
    gl_Position = pc.mvp * vec4(pos, 0.0f, 1.0f);
    uvs = uv;
    colors = color;
}
