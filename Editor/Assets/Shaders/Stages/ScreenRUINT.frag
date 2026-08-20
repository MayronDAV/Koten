#version 450 core
layout(location = 0) out uint o_ID;

layout(location = 0) in vec2 v_TexCoord;
layout(binding = 1) uniform usampler2D u_Texture;



void main()
{
    o_ID = texture(u_Texture, v_TexCoord).r;
}