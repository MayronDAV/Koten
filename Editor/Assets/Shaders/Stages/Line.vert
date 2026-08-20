#version 450 core

layout(location = 0) out flat vec4  vColor;
layout(location = 1) out flat mat4  vTransform;
layout(location = 5) out flat vec4  vStart;
layout(location = 6) out flat vec4  vEnd;
layout(location = 7) out flat float vWidth;

layout(std430, set = 0, binding = 0) uniform Camera {
    mat4 u_ViewProjection;
};

#define MAX_INSTANCES 5000
struct InstanceData {
    mat4 Transform;
    vec4 Start;
    vec4 End;
    vec4 Color;
    float Width;
};
layout(std430, set = 0, binding = 1) uniform u_Instances {
    InstanceData Instances[MAX_INSTANCES];
};

void main()
{
    vColor        = Instances[gl_InstanceIndex].Color;
    vTransform    = Instances[gl_InstanceIndex].Transform;
    vStart        = Instances[gl_InstanceIndex].Start;
    vEnd          = Instances[gl_InstanceIndex].End;
    vWidth        = Instances[gl_InstanceIndex].Width;
    
    vec4 position = (gl_VertexIndex % 2) == 0 ? vTransform * vStart : vTransform * vEnd;
    gl_Position   = u_ViewProjection * position;
}