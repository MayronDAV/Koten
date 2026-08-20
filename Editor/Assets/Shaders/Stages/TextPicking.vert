#version 450 core

layout(location = 0) out flat uint v_PickingID;

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

layout(std430, set = 0, binding = 2) buffer PickingBuffer 
{
    uint Count;
    uint[] PickingIDs;
} b_PickingBuffer;



void main()
{
    vec2 posMin         = Instances[gl_InstanceIndex].Positions.xy;
    vec2 posMax         = Instances[gl_InstanceIndex].Positions.zw;
    vec2 relativePos[4] = vec2[](
        vec2(0.0, 1.0),  // Top Left
        vec2(0.0, 0.0),  // Bottom Left
        vec2(1.0, 1.0),  // Top Right
        vec2(1.0, 0.0)   // Bottom Right
    );
    vec2 rel            = relativePos[gl_VertexIndex % 4];
    vec2 position       = posMin + rel * (posMax - posMin);

    uint count          = b_PickingBuffer.Count;
    int isValid         = int(gl_InstanceIndex < count);
    if (count == 0u || isValid == 0)
    {
        v_PickingID     = 0u; // set to invalid
    }
    else
    {
        v_PickingID     = b_PickingBuffer.PickingIDs[gl_InstanceIndex];
    }

    float zPos          = Instances[gl_InstanceIndex].TexIndex == 0.0 ? -0.001 : 0.0;
    gl_Position         = u_ViewProjection  * Instances[gl_InstanceIndex].Transform * vec4(position, zPos, 1.0);
}