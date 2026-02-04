#version 460 core

layout(location = 0) out vec4 out_FragColor;

layout(push_constant) uniform PerFrameData {
    mat4 proj;
		vec3 colors;
}pc;

void main () {
		out_FragColor = vec4(pc.colors, 1.0f);
}
