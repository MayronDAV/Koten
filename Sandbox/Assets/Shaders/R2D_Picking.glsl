@type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

struct VertexData
{
    vec3 Position;
    vec2 CircleData;
};
layout(location = 0) out VertexData Output;
layout(location = 2) out flat float v_Type;
layout(location = 3) out flat int v_EntityID;

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


layout(std430, set = 0, binding = 2) buffer EntityBuffer 
{
    int Count;
    int[] EnttIDs;
} b_EntityBuffer;


void main()
{
    Output.Position = a_Position;
    Output.CircleData = vec2(Instances[gl_InstanceIndex].Others.zw);
    v_Type = Instances[gl_InstanceIndex].Others.x;

    int count = b_EntityBuffer.Count;
    int isValid = int(gl_InstanceIndex < count);
    v_EntityID = isValid * b_EntityBuffer.EnttIDs[gl_InstanceIndex] + (1 - isValid) * (-1); // -1 is invalid

    gl_Position = u_ViewProjection * Instances[gl_InstanceIndex].Transform * vec4(a_Position, 1.0f);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

@type fragment
#version 450 core

layout(location = 0) out int o_Color;

struct VertexData
{
    vec3 Position;
    vec2 CircleData;
};
layout(location = 0) in VertexData Input;
layout(location = 2) in flat float v_Type;
layout(location = 3) in flat int v_EntityID;



void main()
{
    if (v_Type == 1.0)
    {
        float thickness = Input.CircleData.x;
        float fade = Input.CircleData.y;

        // Calculate distance and fill circle with white
        float distance = 1.0 - length(Input.Position * 2.0);
        float circle = smoothstep(0.0, fade, distance) * smoothstep(thickness + fade, thickness, distance);

        if (circle <= 0.1)
            discard;
    }

    o_Color = v_EntityID;
}