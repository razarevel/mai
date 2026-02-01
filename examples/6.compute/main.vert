#version 460 core

#include <common.sp>

layout(location = 0) out vec2 uv;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec3 color;

layout(buffer_reference, scalar) readonly buffer Matrices{
		mat4 mtx[];
};

struct Vertex {
		float x,y,z;
		float u,v;
		float nx, ny, nz;
};

layout(buffer_reference, scalar) readonly buffer Vertices{
		Vertex in_Vertices[];
};

const vec3 colors[3] = vec3[3](
				vec3(1.0f, 0.0f, 0.0f),
				vec3(0.0f, 1.0f, 0.0f),
				vec3(0.0f, 0.0f, 1.0f)
				);

void main () {
  Vertex vtx = Vertices(bufVerticesId).in_Vertices[gl_VertexIndex];

	mat4 model = Matrices(bufMatricesId).mtx[gl_InstanceIndex];

   const float scale = 10.0;

	gl_Position = viewproj * model * vec4(scale * vtx.x, scale * vtx.y, scale * vtx.z, 1.0);

	mat3 normalMatrix = transpose( inverse(mat3(model)) );

	uv = vec2(vtx.u, vtx.v);
	normal = normalMatrix * vec3(vtx.nx, vtx.ny, vtx.nz);
	color = colors[gl_InstanceIndex % 3];
}
