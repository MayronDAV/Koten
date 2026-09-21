#pragma once
#include "Koten/Core/Base.h"
#include "Texture.h"
#include "Koten/Scene/Entity.h"



namespace KTN
{
    class KTN_API PickingManager
    {
        public:
            static void Init();
            static void Shutdown();

            static void Begin();

            static void Update(uint32_t p_ID, uint32_t p_Width, uint32_t p_Height);

            static PickingID RegisterEntity(Entity p_Entity, bool p_Force = true);

            static uint32_t CreatePickingTarget(uint32_t p_Width, uint32_t p_Height);
            static Entity ReadPixel(uint32_t p_ID, uint32_t p_X, uint32_t p_Y);
            static Entity GetEntityByPickingID(PickingID p_ID);
            static Ref<Texture2D> GetPickingTarget(uint32_t p_ID);

        private:
            struct PickingData
            {
                std::unordered_map<uint32_t, Ref<Texture2D>> PickingTargets;
                std::unordered_map<PickingID, Entity> PickingMap;
                PickingID NextID = 1;
                uint32_t NextPickingTargetID = 1;
            };
            inline static PickingData* s_PickingData;
    };


} // namespace KTN