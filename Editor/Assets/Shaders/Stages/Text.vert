#version 450 core

struct VertexData
{
    vec4 Color;
    vec4 BgColor;
    vec2 UV;
};
layout(location = 0) out VertexData Output;
layout(location = 3) out flat float v_TexIndex;

layout(std430, set = 0, binding = 0) uniform Camera
{
    mat4 u_ViewProjection;
};

#define MAX_INSTANCES 5000
struct InstanceData
{
    mat4 Transform;
    vec4 Positions;
    vec4 Color;
    vec4 BgColor;
    vec4 UV;
    float TexIndex; // 0-31
};
layout(std430, set = 0, binding = 1) uniform u_Instances
{
    InstanceData Instances[MAX_INSTANCES];
};



void main()
{
    vec2 posMin    = Instances[gl_InstanceIndex].Positions.xy;
    vec2 posMax    = Instances[gl_InstanceIndex].Positions.zw;
    vec2 relativePos[4] = vec2[](
        vec2(0.0, 1.0),  // Top Left
        vec2(0.0, 0.0),  // Bottom Left
        vec2(1.0, 1.0),  // Top Right
        vec2(1.0, 0.0)   // Bottom Right
    );
    vec2 rel       = relativePos[gl_VertexIndex % 4];
    vec2 position  = posMin + rel * (posMax - posMin);

    Output.Color   = Instances[gl_InstanceIndex].Color;
    Output.BgColor = Instances[gl_InstanceIndex].BgColor;
    vec2 uvMin     = Instances[gl_InstanceIndex].UV.xy;
    vec2 uvMax     = Instances[gl_InstanceIndex].UV.zw;
    Output.UV      = uvMin + rel * (uvMax - uvMin);

    v_TexIndex     = Instances[gl_InstanceIndex].TexIndex;

    float zPos     = Instances[gl_InstanceIndex].TexIndex == 0.0 ? -0.001 : 0.0;
    gl_Position    = u_ViewProjection  * Instances[gl_InstanceIndex].Transform * vec4(position, zPos, 1.0);
}