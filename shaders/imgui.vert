#version 460 core
layout(location = 0) out vec4 out_color;
layout(location = 1) out vec2 out_uv;

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

struct Vertex {
    float x, y;
    float u, v;
    uint rgba;
};

layout(buffer_reference, scalar) readonly buffer VertexBuffer {
    Vertex vertices[];
};

layout(push_constant) uniform PushConstants {
    vec4 LRTB;
    VertexBuffer vb;
    uint textureId;
    uint samplerId;
} pc;

void main() {
    float L = pc.LRTB.x;
    float R = pc.LRTB.y;
    float T = pc.LRTB.z;
    float B = pc.LRTB.w;
    mat4 proj = mat4(
            2.0 / (R - L), 0.0, 0.0, 0.0,
            0.0, 2.0 / (T - B), 0.0, 0.0,
            0.0, 0.0, -1.0, 0.0,
            (R + L) / (L - R), (T + B) / (B - T), 0.0, 1.0);
    Vertex v = pc.vb.vertices[gl_VertexIndex];
    out_color = unpackUnorm4x8(v.rgba);
    out_uv = vec2(v.u, v.v);
    gl_Position = proj * vec4(v.x, v.y, 0, 1);
}
