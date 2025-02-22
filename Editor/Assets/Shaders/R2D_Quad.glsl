@type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

struct VertexData
{
    vec4 Color;
    vec2 UV;
};
layout(location = 0) out VertexData Output;
layout(location = 3) out flat float v_TexIndex;

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

void main()
{
    vec2 min = Instances[gl_InstanceIndex].UVMin;
    vec2 max = Instances[gl_InstanceIndex].UVMax;

    vec2 uvs[4] = vec2[4](
        vec2( min.x, min.y ),
        vec2( max.x, min.y ),
        vec2( max.x, max.y ),
        vec2( min.x, max.y )
    );


    Output.Color = Instances[gl_InstanceIndex].Color;
    Output.UV = uvs[gl_VertexIndex];
    v_TexIndex = Instances[gl_InstanceIndex].TexIndex;

    gl_Position = u_ViewProjection * Instances[gl_InstanceIndex].Transform * vec4(a_Position, 1.0f);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

@type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

struct VertexData
{
    vec4 Color;
    vec2 UV;
};
layout(location = 0) in VertexData Input;
layout(location = 3) in flat float v_TexIndex;

layout(set = 0, binding = 2) uniform sampler2D u_Textures[32];

void main()
{
    vec4 texColor = Input.Color;

    #define SAMPLE_TEXTURE(index) case index: texColor *= texture(u_Textures[index], Input.UV); break;

    switch(int(v_TexIndex))
    {
        SAMPLE_TEXTURE( 0)
        SAMPLE_TEXTURE( 1)
        SAMPLE_TEXTURE( 2)
        SAMPLE_TEXTURE( 3)
        SAMPLE_TEXTURE( 4)
        SAMPLE_TEXTURE( 5)
        SAMPLE_TEXTURE( 6)
        SAMPLE_TEXTURE( 7)
        SAMPLE_TEXTURE( 8)
        SAMPLE_TEXTURE( 9)
        SAMPLE_TEXTURE(10)
        SAMPLE_TEXTURE(11)
        SAMPLE_TEXTURE(12)
        SAMPLE_TEXTURE(13)
        SAMPLE_TEXTURE(14)
        SAMPLE_TEXTURE(15)
        SAMPLE_TEXTURE(16)
        SAMPLE_TEXTURE(17)
        SAMPLE_TEXTURE(18)
        SAMPLE_TEXTURE(19)
        SAMPLE_TEXTURE(20)
        SAMPLE_TEXTURE(21)
        SAMPLE_TEXTURE(22)
        SAMPLE_TEXTURE(23)
        SAMPLE_TEXTURE(24)
        SAMPLE_TEXTURE(25)
        SAMPLE_TEXTURE(26)
        SAMPLE_TEXTURE(27)
        SAMPLE_TEXTURE(28)
        SAMPLE_TEXTURE(29)
        SAMPLE_TEXTURE(30)
        SAMPLE_TEXTURE(31)
    }

    o_Color = texColor;
}
