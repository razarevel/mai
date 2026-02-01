
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

struct Vertex {
	float x, y, z;
	float u, v;
};

layout(buffer_reference, scalar) readonly buffer Vertices {
	Vertex in_Vertices[];
};

layout(buffer_reference, scalar) readonly buffer PerFrameData {
	mat4 model;
	mat4 view;
	mat4 proj;
	vec4 cameraPos;
	uint texture;
	float tesselationScale;
	Vertices vtx;
};

layout(push_constant) uniform PushConstants {
	PerFrameData pc;
};
