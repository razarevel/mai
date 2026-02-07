#version 460 core
#include <common.sp>

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec2 uvs;

void main () {
		gl_Position = pc.proj * pc.view * pc.model * vec4(pos, 1.0f);
		uvs = uv;
}



