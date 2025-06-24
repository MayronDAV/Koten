@type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

struct VertexData
{
    vec3 Position;
    vec4 Color;
    vec2 UV;
    vec2 CircleData;
};
layout(location = 0) out VertexData Output;
layout(location = 4) out flat float v_TexIndex;
layout(location = 5) out flat float v_Type;

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



void main()
{
    // Get the instance data
    Output.Position = a_Position;
    Output.Color = Instances[gl_InstanceIndex].Color;
    Output.UV = mix(vec2(Instances[gl_InstanceIndex].UV.xy), vec2(Instances[gl_InstanceIndex].UV.zw), a_Position.xy);
    Output.CircleData = vec2(Instances[gl_InstanceIndex].Others.zw);
    v_TexIndex = Instances[gl_InstanceIndex].Others.y;
    v_Type = Instances[gl_InstanceIndex].Others.x;

    gl_Position = u_ViewProjection * Instances[gl_InstanceIndex].Transform * vec4(a_Position, 1.0f);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

@type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

struct VertexData
{
    vec3 Position;
    vec4 Color;
    vec2 UV;
    vec2 CircleData;
};
layout(location = 0) in VertexData Input;
layout(location = 4) in flat float v_TexIndex;
layout(location = 5) in flat float v_Type;

layout(set = 0, binding = 2) uniform sampler2D u_Textures[32];



void main()
{
    float circle = 1.0;
    if (v_Type == 1.0)
    {
        float thickness = Input.CircleData.x;
        float fade = Input.CircleData.y;

        // Calculate distance and fill circle with white
        float distance = 1.0 - length(Input.Position * 2.0);
        circle = smoothstep(0.0, fade, distance) * smoothstep(thickness + fade, thickness, distance);

        if (circle <= 0.1)
            discard;
    }

    vec4 texColor = Input.Color;

    #define SAMPLE_TEXTURE(index) case index: texColor *= texture(u_Textures[index], Input.UV); break;

    switch(int(v_TexIndex)) {
        SAMPLE_TEXTURE(0)  SAMPLE_TEXTURE(1)  SAMPLE_TEXTURE(2)  SAMPLE_TEXTURE(3)
        SAMPLE_TEXTURE(4)  SAMPLE_TEXTURE(5)  SAMPLE_TEXTURE(6)  SAMPLE_TEXTURE(7)
        SAMPLE_TEXTURE(8)  SAMPLE_TEXTURE(9)  SAMPLE_TEXTURE(10) SAMPLE_TEXTURE(11)
        SAMPLE_TEXTURE(12) SAMPLE_TEXTURE(13) SAMPLE_TEXTURE(14) SAMPLE_TEXTURE(15)
        SAMPLE_TEXTURE(16) SAMPLE_TEXTURE(17) SAMPLE_TEXTURE(18) SAMPLE_TEXTURE(19)
        SAMPLE_TEXTURE(20) SAMPLE_TEXTURE(21) SAMPLE_TEXTURE(22) SAMPLE_TEXTURE(23)
        SAMPLE_TEXTURE(24) SAMPLE_TEXTURE(25) SAMPLE_TEXTURE(26) SAMPLE_TEXTURE(27)
        SAMPLE_TEXTURE(28) SAMPLE_TEXTURE(29) SAMPLE_TEXTURE(30) SAMPLE_TEXTURE(31)
    }

    o_Color = texColor;
    o_Color.a *= circle;
}
