#version 460 core

layout(triangles)in;
layout(triangle_strip, max_vertices=3)out;

layout(location = 0) in vec2 uv[];
layout(location = 1) out vec2 uvs;
layout(location = 2) out vec3 barycoords;

in gl_Vertex{
		gl_Position;
}gl_in[];

void main () {
		const vec3 bc[3] = vec3[3](
						vec3(1.0f, 0.0f, 0.0f),
						vec3(0.0f, 1.0f, 0.0f),
						vec3(0.0f, 0.0f, 1.0f)
						);
		for(int i=0; i < 3;i++){
				gl_Position = gl_in[i].gl_Position;
				uvs = uv[i];
				barycoords = bc[i];
				EmitVertex();
		}
		EndPrimitive();
}
