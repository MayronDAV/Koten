#version 450 core
layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec2 v_TexCoord;
layout(set = 0, binding = 1) uniform sampler2D u_Texture;



void main()
{
    vec3 color = texture(u_Texture, v_TexCoord).rgb;
    o_Color    = vec4(color, 1.0);
}