#version 450 core

layout(location = 0) out uint o_Color;

layout(location = 0) in flat uint v_PickingID;



void main()
{
    o_Color = v_PickingID;
}