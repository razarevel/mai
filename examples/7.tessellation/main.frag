#version 460 core

#include <common.sp>

#extension GL_EXT_nonuniform_qualifier : require

layout(set = 0, binding = 0) uniform texture2D kTextures2D[];
layout(set = 0, binding = 1) uniform sampler kSamplers[];

layout(location = 0) in vec2 uvs;
layout(location = 1) in vec3 barycoords;

layout(location = 0) out vec4 out_FragColor;

float edgeFactor(float thickness) {
	vec3 a3 = smoothstep( vec3( 0.0 ), fwidth(barycoords) * thickness, barycoords);
	return min( min( a3.x, a3.y ), a3.z );
}

vec4 textureBindless2D(uint textureid, uint samplerid, vec2 uv) {
    return texture(nonuniformEXT(sampler2D(kTextures2D[textureid], kSamplers[samplerid])), uv);
}

void main() {
	vec4 color = textureBindless2D(pc.texture, 0, uvs);
	out_FragColor = mix( vec4(0.1), color, edgeFactor(0.75) );
}
