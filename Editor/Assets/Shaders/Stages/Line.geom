#version 450 core

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

layout(location = 0) in flat vec4  vColor[];
layout(location = 1) in flat mat4  vTransform[];
layout(location = 5) in flat vec4  vStart[];
layout(location = 6) in flat vec4  vEnd[];
layout(location = 7) in flat float vWidth[];

layout(location = 0) out vec4 fColor;

layout(std430, set = 0, binding = 0) uniform Camera {
    mat4 u_ViewProjection;
};

void main() 
{
    vec4 worldStart = vTransform[0] * vStart[0];
    vec4 worldEnd   = vTransform[0] * vEnd[0];
    
    vec3 lineDir    = normalize(worldEnd.xyz - worldStart.xyz);
    
    vec3 up         = abs(lineDir.y) > 0.99 ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 1.0, 0.0);
    vec3 transUp    = normalize(mat3(vTransform[0]) * up);
    vec3 right      = normalize(cross(lineDir, transUp));
    vec3 actualUp   = normalize(cross(right, lineDir));
    
    vec3 offset     = actualUp * vWidth[0] * 0.5;
    
    fColor          = vColor[0];

    gl_Position     = u_ViewProjection * vec4(worldStart.xyz - offset, worldStart.w);
    EmitVertex();

    gl_Position     = u_ViewProjection * vec4(worldStart.xyz + offset, worldStart.w);
    EmitVertex();

    gl_Position     = u_ViewProjection * vec4(worldEnd.xyz - offset, worldEnd.w);
    EmitVertex();

    gl_Position     = u_ViewProjection * vec4(worldEnd.xyz + offset, worldEnd.w);
    EmitVertex();
    
    EndPrimitive();
}