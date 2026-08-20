#version 450 core

layout(location = 0) out uint o_Color;

struct VertexData
{
    vec3 Position;
    vec2 CircleData;
};
layout(location = 0) in VertexData Input;
layout(location = 2) in flat float v_Type;
layout(location = 3) in flat uint v_PickingID;



void main()
{
    if (v_Type == 1.0)
    {
        float thickness = Input.CircleData.x;
        float fade      = Input.CircleData.y;

        // Calculate distance and fill circle with white
        float distance  = 1.0 - length(Input.Position * 2.0);
        float circle    = smoothstep(0.0, fade, distance) * smoothstep(thickness + fade, thickness, distance);

        if (circle <= 0.1)
            discard;
    }

    o_Color = v_PickingID;
}