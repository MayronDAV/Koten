@type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

struct VertexData
{
    vec4 Color;
};
layout(location = 0) out VertexData Output;

layout(std430, set = 0, binding = 0) uniform Camera
{
    mat4 u_ViewProjection;
};


#define MAX_INSTANCES 5000
struct InstanceData
{
    vec4 Color;
    vec4 Start;
    vec4 End;
    float Width;
};
layout(std430, set = 0, binding = 1) uniform u_Instances
{
    InstanceData Instances[MAX_INSTANCES];
};



void main()
{
    Output.Color = Instances[gl_InstanceIndex].Color;

     bool isStart = (gl_VertexIndex % 2) == 0;
    vec4 position = isStart ? Instances[gl_InstanceIndex].Start :  Instances[gl_InstanceIndex].End;

    gl_Position = u_ViewProjection * position;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

@type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

struct VertexData
{
    vec4 Color;
};
layout(location = 0) in VertexData Input;



void main()
{
    o_Color = Input.Color;
}
