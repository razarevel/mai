#version 460 core

layout(push_constant) uniform PerFrameData {
    mat4 proj;
    uint textureId;
    float x;
    float y;
    float width;
    float height;
    float alphaScale;
} pc;

const vec2 pos[4] = vec2[4](
        vec2(0.5, -0.5),
        vec2(0.5, 0.5),
        vec2(-0.5, -0.5),
        vec2(-0.5, 0.5)
    );

layout(location = 0) out vec2 uv;

void main() {
    uv = pos[gl_VertexIndex] + vec2(0.5);
    vec2 p = pos[gl_VertexIndex] * vec2(pc.width, pc.height) + vec2(pc.x, pc.y);
    gl_Position = pc.proj * vec4(p, 0.0f, 1.0f);
}
