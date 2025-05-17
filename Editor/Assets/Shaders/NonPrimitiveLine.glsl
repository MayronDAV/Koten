@type vertex
#version 450 core

layout(location = 0) out flat mat4 vTransform;
layout(location = 4) out flat vec4 vStart;
layout(location = 5) out flat vec4 vEnd;
layout(location = 6) out flat vec4 vColor;
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
    vTransform = Instances[gl_InstanceIndex].Transform;
    vStart = Instances[gl_InstanceIndex].Start;
    vEnd = Instances[gl_InstanceIndex].End;
    vColor = Instances[gl_InstanceIndex].Color;
    vWidth = Instances[gl_InstanceIndex].Width;
    
    gl_Position = vec4(0.0); // Dummy position
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
@type geometry
#version 450 core

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

layout(location = 0) in flat mat4 vTransform[];
layout(location = 4) in flat vec4 vStart[];
layout(location = 5) in flat vec4 vEnd[];
layout(location = 6) in flat vec4 vColor[];
layout(location = 7) in flat float vWidth[];

layout(location = 0) out vec4 fColor;

layout(std430, set = 0, binding = 0) uniform Camera {
    mat4 u_ViewProjection;
};

void main() 
{
    vec4 worldStart = vTransform[0] * vStart[0];
    vec4 worldEnd = vTransform[0] * vEnd[0];
    
    vec3 lineDir = normalize(worldEnd.xyz - worldStart.xyz);
    
    vec3 up = abs(lineDir.y) > 0.99 ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 1.0, 0.0);
    vec3 transUp = normalize(mat3(vTransform[0]) * up);
    vec3 right = normalize(cross(lineDir, transUp));
    vec3 actualUp = normalize(cross(right, lineDir));
    
    vec3 offset = actualUp * vWidth[0] * 0.5;
    
    fColor = vColor[0];

    gl_Position = u_ViewProjection * vec4(worldStart.xyz - offset, worldStart.w);
    EmitVertex();

    gl_Position = u_ViewProjection * vec4(worldStart.xyz + offset, worldStart.w);
    EmitVertex();

    gl_Position = u_ViewProjection * vec4(worldEnd.xyz - offset, worldEnd.w);
    EmitVertex();

    gl_Position = u_ViewProjection * vec4(worldEnd.xyz + offset, worldEnd.w);
    EmitVertex();
    
    EndPrimitive();
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
@type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec4 fColor;

void main() 
{
    o_Color = fColor;
}