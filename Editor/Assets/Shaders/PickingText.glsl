@type vertex
#version 450 core

layout(location = 0) out flat int v_EntityID;

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

layout(std430, set = 0, binding = 2) buffer EntityBuffer 
{
    int Count;
    int[] EnttIDs;
} b_EntityBuffer;



void main()
{
    vec2 posMin = Instances[gl_InstanceIndex].Positions.xy;
    vec2 posMax = Instances[gl_InstanceIndex].Positions.zw;
    vec2 relativePos[4] = vec2[](
        vec2(0.0, 1.0),  // Top Left
        vec2(0.0, 0.0),  // Bottom Left
        vec2(1.0, 1.0),  // Top Right
        vec2(1.0, 0.0)   // Bottom Right
    );
    vec2 rel = relativePos[gl_VertexIndex % 4];
    vec2 position = posMin + rel * (posMax - posMin);

    int count = b_EntityBuffer.Count;
    int isValid = int(gl_InstanceIndex < count);
    v_EntityID = isValid * b_EntityBuffer.EnttIDs[gl_InstanceIndex] + (1 - isValid) * (-1); // -1 is invalid

    float zPos = Instances[gl_InstanceIndex].TexIndex == 0.0 ? -0.001 : 0.0;
    gl_Position = u_ViewProjection  * Instances[gl_InstanceIndex].Transform * vec4(position, zPos, 1.0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

@type fragment
#version 450 core

layout(location = 0) out int o_Color;

layout(location = 0) in flat int v_EntityID;



void main()
{
    o_Color = v_EntityID;
}