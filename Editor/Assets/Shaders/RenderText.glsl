@type vertex
#version 450 core

struct VertexData
{
    vec4 Color;
    vec2 UV;
};
layout(location = 0) out VertexData Output;
layout(location = 2) out flat float v_TexIndex;

layout(std430, set = 0, binding = 0) uniform Camera
{
    mat4 u_ViewProjection;
};

#define MAX_INSTANCES 5000
struct InstanceData
{
    mat4 Transform;
    vec4 Positions;
    vec4 Color;
    vec4 UV;
    float TexIndex;
};
layout(std430, set = 0, binding = 1) uniform u_Instances
{
    InstanceData Instances[MAX_INSTANCES];
};



void main()
{
    vec2 posMin = Instances[gl_InstanceIndex].Positions.xy;
    vec2 posMax = Instances[gl_InstanceIndex].Positions.zw;
    vec2 relativePos[4] = vec2[](
        vec2(0.0, 1.0),  // Top Left
        vec2(0.0, 0.0),  // Bottom Left
        vec2(1.0, 1.0),  // Top Right
        vec2(1.0, 0.0)   // Bottom Right
    );
    vec2 rel = relativePos[gl_VertexIndex % 4];
    vec2 position = posMin + rel * (posMax - posMin);

    Output.Color = Instances[gl_InstanceIndex].Color;
    vec2 uvMin = Instances[gl_InstanceIndex].UV.xy;
    vec2 uvMax = Instances[gl_InstanceIndex].UV.zw;
    Output.UV = uvMin + rel * (uvMax - uvMin);

    v_TexIndex = Instances[gl_InstanceIndex].TexIndex;

    gl_Position = u_ViewProjection  * Instances[gl_InstanceIndex].Transform * vec4(position, 0.0, 1.0);
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
layout(location = 2) in flat float v_TexIndex;

layout(set = 0, binding = 2) uniform sampler2D u_FontAtlasTextures[32];



float ScreenPxRange(sampler2D p_Texture) 
{
	const float pxRange = 2.0; // set to distance field's pixel range
    vec2 unitRange = vec2(pxRange) / vec2(textureSize(p_Texture, 0));
    vec2 screenTexSize = vec2(1.0) / fwidth(Input.UV);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

float median(float r, float g, float b) 
{
    return max(min(r, g), min(max(r, g), b));
}


void main()
{
    vec3 msd;
    float screenPxDistance;
    #define SAMPLE_TEXTURE(index) case index: msd = texture(u_FontAtlasTextures[index], Input.UV).rgb; screenPxDistance = ScreenPxRange(u_FontAtlasTextures[index]); break;

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

    float sd = median(msd.r, msd.g, msd.b);
    screenPxDistance *= (sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
	if (opacity == 0.0)
		discard;

	vec4 bgColor = vec4(0.0);
    o_Color = mix(bgColor, Input.Color, opacity);
	if (o_Color.a == 0.0)
		discard;
}