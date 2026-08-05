@type vertex
#version 450 core

layout(location = 0) out vec2 v_TexCoord;

out gl_PerVertex
{
	vec4 gl_Position;
};


void main()
{
	vec2 positions[6] = vec2[](
		vec2(-1.0, -1.0), vec2( 1.0, -1.0), vec2(-1.0,  1.0),
		vec2(-1.0,  1.0), vec2( 1.0, -1.0), vec2( 1.0,  1.0)
	);

	vec2 texCoords[6] = vec2[](
		vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(0.0, 1.0),
		vec2(0.0, 1.0), vec2(1.0, 0.0), vec2(1.0, 1.0)
	);

	gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
	v_TexCoord = texCoords[gl_VertexIndex];
}

@type fragment
#version 450 core

layout(location = 0) in vec2 v_TexCoord;
layout(binding = 1) uniform usampler2D u_Texture;

layout(location = 0) out uint o_ID;

void main()
{
    o_ID = texture(u_Texture, v_TexCoord).r;
}