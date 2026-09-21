#include "ktnpch.h"
#include "Koten/OS/Input.h"



namespace KTN
{
    glm::vec2 Input::GetMousePosition()
    {
        KTN_PROFILE_FUNCTION_LOW();

        POINT point;
        if (!GetCursorPos(&point))
            return { 0.0f, 0.0f };

        return {
            static_cast<float>(point.x),
            static_cast<float>(point.y)
        };
    }

} // namespace KTN