#version 460 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec2 uvs;

layout(push_constant) uniform PerFrameData {
    mat4 proj;
		vec3 color;
    uint textureId;
}pc;

void main() {
		gl_Position = pc.proj * vec4(pos, 0.0f, 1.0f);
		uvs = uv;
}
