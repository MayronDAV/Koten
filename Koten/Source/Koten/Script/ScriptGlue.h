#pragma once
#include "Koten/Core/Base.h"

// Services
#include "Services/WindowService.h"



namespace KTN
{
    class KTN_API ScriptGlue
    {
        public:
            static void RegisterComponents();
            static void RegisterFunctions();

            static void Initialize(Unique<WindowService> p_WindowService = nullptr);
            static void Shutdown();
    };

} // namespace KTN