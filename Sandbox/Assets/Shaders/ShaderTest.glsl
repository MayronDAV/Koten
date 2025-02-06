@type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_Texcoord;
layout(location = 2) in vec3 a_Color;

struct VertexData
{
	vec4 Color;
	vec2 Texcoord;
};
layout(location = 0) out VertexData Output;


layout(push_constant) uniform PushConsts
{
	mat4 Matrix;
};


void main()
{
	Output.Color	= vec4(a_Color, 1.0);
	Output.Texcoord = a_Texcoord;

	gl_Position		= Matrix * vec4(a_Position, 1.0f);
}


@type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

struct VertexData
{
	vec4 Color;
	vec2 Texcoord;
};
layout(location = 0) in VertexData Input;


layout(set = 0, binding = 0) uniform sampler2D u_Texture;



void main()
{
	o_Color = Input.Color * texture(u_Texture, Input.Texcoord);
}