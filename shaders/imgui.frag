#version 460 core

#extension GL_EXT_nonuniform_qualifier : require
layout(set = 0, binding = 0) uniform texture2D kTextures2D[];
layout(set = 0, binding = 1) uniform sampler kSamplers[];

layout(location = 0) in vec4 in_color;
layout(location = 1) in vec2 in_uv;

layout(location = 0) out vec4 out_color;

layout(constant_id = 0) const bool kNonLinearColorSpace = false;

layout(push_constant) uniform PushConstants {
    vec4 LRTB;
    vec2 vb;
    uint textureId;
    uint samplerId;
} pc;

vec4 textureBindless2D(uint textureid, uint samplerid, vec2 uv) {
    return texture(nonuniformEXT(sampler2D(kTextures2D[textureid], kSamplers[samplerid])), uv);
}

void main() {
    vec4 c = in_color * textureBindless2D(pc.textureId, pc.samplerId, in_uv);
    // Render UI in linear color space to sRGB framebuffer.
    out_color = kNonLinearColorSpace ? vec4(pow(c.rgb, vec3(2.2)), c.a) : c;
}
