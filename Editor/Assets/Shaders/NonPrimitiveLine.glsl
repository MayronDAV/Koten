@type vertex
#version 450 core

layout(location = 0) out flat mat4 vStart;
layout(location = 4) out flat mat4 vEnd;
layout(location = 8) out flat vec4 vColor;
layout(location = 9) out flat float vWidth;

layout(std430, set = 0, binding = 0) uniform Camera {
    mat4 u_ViewProjection;
};

#define MAX_INSTANCES 5000
struct InstanceData {
    mat4 Start;
    mat4 End;
    vec4 Color;
    float Width;
};
layout(std430, set = 0, binding = 1) uniform u_Instances {
    InstanceData Instances[MAX_INSTANCES];
};

void main()
{
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

layout(location = 0) in flat mat4 vStart[];
layout(location = 4) in flat mat4 vEnd[];
layout(location = 8) in flat vec4 vColor[];
layout(location = 9) in flat float vWidth[];

layout(location = 0) out vec4 fColor;

layout(std430, set = 0, binding = 0) uniform Camera {
    mat4 u_ViewProjection;
};

void main() 
{
    vec3 baseStart = vec3(-0.5, 0.0, 0.0);
    vec3 baseEnd = vec3(0.5, 0.0, 0.0);
    
    vec4 worldStart = vStart[0] * vec4(baseStart, 1.0);
    vec4 worldEnd = vEnd[0] * vec4(baseEnd, 1.0);
    
    vec3 lineDir = normalize(worldEnd.xyz - worldStart.xyz);
    
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 startUp = normalize(mat3(vStart[0]) * up);
    vec3 endUp = normalize(mat3(vEnd[0]) * up);

    vec3 startRight = normalize(cross(lineDir, startUp));
    vec3 endRight = normalize(cross(lineDir, endUp));

    vec3 actualStartUp = normalize(cross(startRight, lineDir));
    vec3 actualEndUp = normalize(cross(endRight, lineDir));
    
    vec3 startOffset = actualStartUp * vWidth[0] * 0.5;
    vec3 endOffset = actualEndUp * vWidth[0] * 0.5;
    
    fColor = vColor[0];

    gl_Position = u_ViewProjection * vec4(worldStart.xyz - startOffset, 1.0);
    EmitVertex();

    gl_Position = u_ViewProjection * vec4(worldStart.xyz + startOffset, 1.0);
    EmitVertex();

    gl_Position = u_ViewProjection * vec4(worldEnd.xyz - endOffset, 1.0);
    EmitVertex();

    gl_Position = u_ViewProjection * vec4(worldEnd.xyz + endOffset, 1.0);
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