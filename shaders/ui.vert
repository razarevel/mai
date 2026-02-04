#version 460 core

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

struct Vertex {
		float x,y,z;
};

layout(buffer_reference, scalar) readonly buffer Vertices {
	Vertex in_Vertices[];
};

layout(push_constant) uniform PerFrameData {
    mat4 proj;
		vec3 colors;
		Vertices vtx;
}pc;

vec3 getPosition(int i) {
	return vec3(pc.vtx.in_Vertices[i].x, pc.vtx.in_Vertices[i].y, pc.vtx.in_Vertices[i].y);
}

void main() {
		int i = gl_VertexIndex;
		gl_Position = pc.proj * vec4(getPosition(i), 1.0f);
}


