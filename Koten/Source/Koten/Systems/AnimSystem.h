#pragma once
#include "Koten/Scene/System.h"
#include "Koten/Scene/Entity.h"
#include "Koten/Asset/AnimationControllerImporter.h"
#include "Koten/Asset/AnimationImporter.h"



namespace KTN
{
    class KTN_API AnimSystem : public System
    {
        public:
            AnimSystem() = default;
            ~AnimSystem() override = default;

            bool OnStart(Scene* p_Scene) override;
            bool OnStop(Scene* p_Scene) override;

            bool OnInit() override;
            void OnUpdate(Scene* p_Scene) override;
    };

} // namespace KTN