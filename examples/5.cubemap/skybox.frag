#version 460 core

#include <common.sp>

#extension GL_EXT_nonuniform_qualifier:require

layout(location = 0) in vec3 dir;
layout(location = 0) out vec4 out_FragColor;

layout(set = 0, binding = 1) uniform sampler kSamplers[];
layout(set = 0, binding = 2) uniform textureCube kCubemaps[];

vec4 textureBindlessCube(uint textureid, uint samplerid, vec3 uv) {
    return texture(nonuniformEXT(
            samplerCube(kCubemaps[textureid], kSamplers[samplerid])), uv);
}

void main() {
    out_FragColor = textureBindlessCube(pc.texCube, 0, dir);
}
