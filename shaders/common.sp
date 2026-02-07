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
