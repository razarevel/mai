#version 460 core

#include <common.sp>

layout(location = 0) in vec2 uv_in[];
layout(location = 1) in vec3 worldPos_in[];

in gl_Vertex{
		vec4 gl_Position;
}gl_in[];

out gl_Vertex{
		vec4 gl_Position;
}gl_out[];

struct Vertex{
		vec2 uv;
};

layout(location = 0) out Vertex Out[];

float getTessLevel(float distance0, float distance1){
		const float distanceScale1 = 1.2;
		const float distanceScale2 = 1.7;
		const float avgDistance = (distance1 + distance2) / (2.0 * pc.tesselationScale);
		
		if(avgDistance <= distanceScale1) return 5.0;
		if(avgDistance <= distanceScale2) return 3.0f;
}

void main () {
		gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
		Out[gl_InvocationID].uv = uv_in[gl_InvocationID];

		vec3 c = pc.cameraPos.xyz;

		float eyeToVertexDistance0 = distance(c, wordlPos_in[0]);
		float eyeToVertexDistance1 = distance(c, wordlPos_in[1]);
		float eyeToVertexDistance2 = distance(c, wordlPos_in[2]);

		gl_TessLevelOuter[0] = getTessLevel(eyeToVertexDistance1, eyeToVertexDistance2);
		gl_TessLevelOuter[1] = getTessLevel(eyeToVertexDistance2, eyeToVertexDistance0);
		gl_TessLevelOuter[2] = getTessLevel(eyeToVertexDistance0, eyeToVertexDistance1);
		gl_TessLevelInner[0] = gl_TessLevelOuter[2];
}
