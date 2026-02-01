
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_buffer_reference_uvec2 : require

layout(push_constant) uniform PerFrameData{
		mat4 viewproj;
		uint textureId;
		uvec2 bufPosAngleId;
		uvec2 bufMatricesId;
		uvec2 bufVerticesId;
		float time;
};

layout(buffer_reference, scalar) readonly buffer Position{
		vec4 pos[];
};
