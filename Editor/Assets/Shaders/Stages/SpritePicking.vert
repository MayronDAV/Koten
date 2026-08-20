#version 450 core

layout(location = 0) in vec3 a_Position;

struct VertexData
{
    vec3 Position;
    vec2 CircleData;
};
layout(location = 0) out VertexData Output;
layout(location = 2) out flat float v_Type;
layout(location = 3) out flat uint v_PickingID;

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


layout(std430, set = 0, binding = 2) buffer PickingBuffer 
{
    uint Count;
    uint[] PickingIDs;
} b_PickingBuffer;


void main()
{
    Output.Position   = a_Position;
    Output.CircleData = vec2(Instances[gl_InstanceIndex].Others.zw);
    v_Type            = Instances[gl_InstanceIndex].Others.x;

    uint count        = b_PickingBuffer.Count;
    int isValid       = int(gl_InstanceIndex < count);
    if (count == 0u || isValid == 0)
    {
        v_PickingID   = 0u; // set to invalid
    }
    else
    {
        v_PickingID   = b_PickingBuffer.PickingIDs[gl_InstanceIndex];
    }

    gl_Position       = u_ViewProjection * Instances[gl_InstanceIndex].Transform * vec4(a_Position, 1.0f);
}