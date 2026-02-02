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

#extension GL_EXT_nonuniform_qualifier:require
layout(set = 0, binding = 0) uniform texture2D kTextures2D[];
layout(set = 0, binding = 1) uniform sampler kSamplers[];

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 out_FragColor;

vec4 textureBindless2D(uint textureid, uint samplerid, vec2 uv) {
    return texture(nonuniformEXT(
            sampler2D(kTextures2D[textureid], kSamplers[samplerid])), uv);
}

void main() {
    out_FragColor = textureBindless2D(pc.textureId, 0, uv) * vec4(vec3(1.0), pc.alphaScale);
}
