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

layout(location = 0) out vec4 o_Color;
layout(location = 0) in vec2 v_TexCoord;

layout(set = 0, binding = 1) uniform sampler2D u_Texture;


void main()
{
	vec3 color		= texture(u_Texture, v_TexCoord).rgb;
	o_Color			= vec4(color, 1.0);
}
