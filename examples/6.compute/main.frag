#version 460 core
#extension GL_EXT_nonuniform_qualifier : require

layout(set = 0, binding = 0) uniform texture2D kTextures2D[];
layout(set = 0, binding = 1) uniform sampler kSamplers[];

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;

layout(push_constant) uniform PerFrameData{
		mat4 viewporj;
		uint textureId;
};

vec4 textureBindless2D(uint textureid, uint samplerid, vec2 uv) {
    return texture(nonuniformEXT(sampler2D(kTextures2D[textureid], kSamplers[samplerid])), uv);
}

layout(location = 0) out vec4 out_FragColor;

void main () {
		vec3 n = normalize(normal);
		vec3 l = normalize(vec3(1.0f, 0.0,1.0f));
		float NdotL = clamp(dot(n,l), 0.3, 1.0f);
		out_FragColor = textureBindless2D(textureId, 0, uv) * NdotL * vec4(color, 1.0f);
}
