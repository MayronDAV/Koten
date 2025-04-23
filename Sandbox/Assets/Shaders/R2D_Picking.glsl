@type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

layout(location = 0) out flat int v_EntityID;

layout(std430, set = 0, binding = 0) uniform Camera
{
    mat4 u_ViewProjection;
};


#define MAX_INSTANCES 5000
struct InstanceData
{
    mat4  Transform;
    vec4  Color;
    vec2 UVMin;
    vec2 UVMax;
    float TexIndex;
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
    int count = b_EntityBuffer.Count;
    int isValid = int(gl_InstanceIndex < count);
    v_EntityID = isValid * b_EntityBuffer.EnttIDs[gl_InstanceIndex] + (1 - isValid) * (-1); // -1 is invalid

    gl_Position = u_ViewProjection * Instances[gl_InstanceIndex].Transform * vec4(a_Position, 1.0f);
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