@type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Color;

struct VertexData
{
	vec4 Color;
};
layout(location = 0) out VertexData Output;


layout(push_constant) uniform PushConsts
{
	mat4 Matrix;
};


void main()
{
	Output.Color	= vec4(a_Color, 1.0);

	gl_Position		= Matrix * vec4(a_Position, 1.0f);
}


@type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

struct VertexData
{
	vec4 Color;
};
layout(location = 0) in VertexData Input;

layout(push_constant) uniform PushConsts
{
	mat4 Matrix;
};

void main()
{
	o_Color = Input.Color;
}