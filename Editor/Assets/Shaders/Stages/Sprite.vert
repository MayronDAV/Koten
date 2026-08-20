#version 450 core

layout(location = 0) in vec3 a_Position;

struct VertexData
{
    vec3 Position;
    vec4 Color;
    vec2 UV;
    vec2 CircleData;
};
layout(location = 0) out VertexData Output;
layout(location = 4) out flat float v_TexIndex;
layout(location = 5) out flat float v_Type;

layout(std430, set = 0, binding = 0) uniform Camera
{
    mat4 u_ViewProjection;
};

#define MAX_INSTANCES 5000
struct InstanceData
{
    mat4 Transform;
    vec4 Color;
    vec4 UV;
    vec4 Others;
};
layout(std430, set = 0, binding = 1) uniform u_Instances
{
    InstanceData Instances[MAX_INSTANCES];
};



void main()
{
    // Get the instance data
    Output.Position   = a_Position;
    Output.Color      = Instances[gl_InstanceIndex].Color;

    vec2 uvVertex     = a_Position.xy + vec2(0.5);

    vec2 minUV        = Instances[gl_InstanceIndex].UV.xy;
    vec2 maxUV        = Instances[gl_InstanceIndex].UV.zw;

    vec2 uvSize       = maxUV - minUV;
    Output.UV         = minUV + uvVertex * uvSize;

    Output.CircleData = vec2(Instances[gl_InstanceIndex].Others.zw);
    v_TexIndex        = Instances[gl_InstanceIndex].Others.y;
    v_Type            = Instances[gl_InstanceIndex].Others.x;

    gl_Position       = u_ViewProjection * Instances[gl_InstanceIndex].Transform * vec4(a_Position, 1.0f);
}