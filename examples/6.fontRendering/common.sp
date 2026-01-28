#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

layout(buffer_reference, scalar) readonly buffer PerFrameData {
    mat4 mvp;
    uint tex;
};

layout(push_constant) uniform PushConstants {
    PerFrameData pc;
};
