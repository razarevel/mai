#version 460 core

layout(triangles, equal_spacing,ccw) in;

struct Vertex{
		vec2 uv;
};

in gl_Vertex{
		vec3 gl_Position;
}gl_in[];

layout(location = 0) in Vertex In[];

out gl_Vertex{
		vec3 gl_Position;
};

layout(location = 0) out vec2 uv;

vec2 interpolate2(in vec2 v0, in vec2 v1, in vec2 v2) {
		return v0 * gl_TessCoord.x + v1 *gl_TessCoord.y + v2  * gl_TessCoord.y;
}

vec4 interpolate4(in vec4 v0, in vec4 v1, in vec4 v2) {
		return v0 * gl_TessCoord.x + v1 *gl_TessCoord.y + v2  * gl_TessCoord.y;
}

void main () {
		gl_Position = interpolate4(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position);
		uv = interpolate2(In[0].uv, In[1].uv, In[2].uv);
}
