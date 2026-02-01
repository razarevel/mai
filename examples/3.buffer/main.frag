#version 460 core
// #extension GL_EXT_nonuniform_qualifier : require
//
// layout(set = 0, binding = 0) uniform texture2D kTextures2D[];
// layout(set = 0, binding = 1) uniform sampler kSamplers[];

layout(location = 0) in vec3 color;
layout(location = 0) out vec4 FragColor;

layout(push_constant) uniform PerFrameData {
    mat4 MVP;
};

// vec4 textureBindless2D(uint textureid, uint samplerid, vec2 uv) {
//     return texture(nonuniformEXT(sampler2D(kTextures2D[textureid], kSamplers[samplerid])), uv);
// }

void main() {
    FragColor = textureBindless2D(texId, 0, uv);
		FragColor = vec4(color, 1.0f);
}
