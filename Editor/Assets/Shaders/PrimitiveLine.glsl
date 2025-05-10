@type vertex
#version 450 core

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
struct InstanceData {
    mat4 Start;
    mat4 End;
    vec4 Color;
    float Width;
};
layout(std430, set = 0, binding = 1) uniform u_Instances
{
    InstanceData Instances[MAX_INSTANCES];
};



void main()
{
    Output.Color = Instances[gl_InstanceIndex].Color;

    vec3 baseStart = vec3(-0.5, 0.0, 0.0);
    vec3 baseEnd = vec3(0.5, 0.0, 0.0);

    vec4 position = (gl_VertexIndex % 2) == 0 ? Instances[gl_InstanceIndex].Start * vec4(baseStart, 1.0) : Instances[gl_InstanceIndex].End * vec4(baseEnd, 1.0);

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
