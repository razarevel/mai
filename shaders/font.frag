#version 460 core
#extension GL_EXT_nonuniform_qualifier : require

layout(set = 0, binding = 0) uniform texture2D kTextures2D[];
layout(set = 0, binding = 1) uniform sampler kSamplers[];

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 colors;
layout(location = 0) out vec4 FragColor;

layout(push_constant) uniform PerFrameData {
    mat4 proj;
    uint textureId;
}pc;


vec4 textureBindless2D(uint textureid, uint samplerid, vec2 uv) {
    return texture(nonuniformEXT(sampler2D(kTextures2D[textureid], kSamplers[samplerid])), uv);
}

void main() {
		vec4 tex = vec4(textureBindless2D(pc.textureId, 0, uv).r);
    FragColor = vec4(colors, 1.0f) *tex;
}
