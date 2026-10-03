#pragma once
#include "Koten/Core/Base.h"


namespace KTN
{
    class Scene;

    class KTN_API System
    {
        public:
            System() = default;
            virtual ~System() = default;

            virtual bool OnStart(Scene* p_Scene) { return true; }
            virtual bool OnStop(Scene* p_Scene) { return true; }
            virtual bool OnInit() { return true; }
            virtual void OnUpdate(Scene* p_Scene) {}
            virtual void OnViewportUpdate(Scene* p_Scene, const Ref<ViewportContext>& p_Viewport) {}
            virtual void OnViewportRender(Scene* p_Scene, const Ref<ViewportContext>& p_Viewport) {}

            virtual void SetPaused(bool p_Value) { m_Paused = p_Value; }

            inline const char* GetName() const
            {
                return m_DebugName;
            }

        protected:
            const char* m_DebugName = "System";
            bool m_Paused = false;
    };

} // namespace KTN
