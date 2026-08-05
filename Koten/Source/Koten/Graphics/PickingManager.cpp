#include "ktnpch.h"
#include "PickingManager.h"
#include "RendererCommand.h"


namespace KTN
{
    void PickingManager::Init()
    {
        KTN_PROFILE_FUNCTION();

        s_PickingData = new PickingData();
    }

    void PickingManager::Shutdown()
    {
        KTN_PROFILE_FUNCTION();

        delete s_PickingData;
    }

    void PickingManager::Begin()
    {
        KTN_PROFILE_FUNCTION();

        s_PickingData->PickingMap.clear();
        s_PickingData->NextID = 1;
    }

    void PickingManager::Update(uint32_t p_ID, uint32_t p_Width, uint32_t p_Height)
    {
                KTN_PROFILE_FUNCTION();

        if (!Engine::Get().GetSettings().MousePicking)
            return;

        if (s_PickingData->PickingTargets.find(p_ID) == s_PickingData->PickingTargets.end())
            return;

        TextureSpecification tspec          = {};
        tspec.Usage                         = TextureUsage::TEXTURE_COLOR_ATTACHMENT;
        tspec.Format                        = TextureFormat::R32_UINT;
        tspec.Width                         = p_Width;
        tspec.Height                        = p_Height;
        tspec.GenerateMips                  = false;
        tspec.AnisotropyEnable              = false;
        tspec.Samples                       = 1;
        tspec.DebugName                     = "PickingManager-Target " + std::to_string(p_ID);

        s_PickingData->PickingTargets[p_ID] = Texture2D::Get(tspec);
    }

    PickingID PickingManager::RegisterEntity(Entity p_Entity)
    {
        KTN_PROFILE_FUNCTION();

        if (!Engine::Get().GetSettings().MousePicking)
            return INVALID_PICKING_ID;

        PickingID id                  = s_PickingData->NextID++;
        s_PickingData->PickingMap[id] = p_Entity;

        return id;
    }

    uint32_t PickingManager::CreatePickingTarget(uint32_t p_Width, uint32_t p_Height)
    {
        KTN_PROFILE_FUNCTION();

        if (!Engine::Get().GetSettings().MousePicking)
            return 0;

        uint32_t id                       = s_PickingData->NextPickingTargetID++;

        TextureSpecification tspec        = {};
        tspec.Usage                       = TextureUsage::TEXTURE_COLOR_ATTACHMENT;
        tspec.Format                      = TextureFormat::R32_UINT;
        tspec.Width                       = p_Width;
        tspec.Height                      = p_Height;
        tspec.GenerateMips                = false;
        tspec.AnisotropyEnable            = false;
        tspec.Samples                     = 1;
        tspec.DebugName                   = "PickingManager-Target " + std::to_string(id);

        s_PickingData->PickingTargets[id] = Texture2D::Get(tspec);
        return id;
    }

    Entity PickingManager::ReadPixel(uint32_t p_ID, uint32_t p_X, uint32_t p_Y)
    {
        KTN_PROFILE_FUNCTION();

        if (!Engine::Get().GetSettings().MousePicking || !s_PickingData->PickingTargets[p_ID])
            return {};

        void* pixelData  = RendererCommand::ReadPixel(s_PickingData->PickingTargets[p_ID], p_X, p_Y);
        if (pixelData)
        {
            PickingID id = static_cast<uint32_t>(reinterpret_cast<std::uintptr_t>(pixelData));
            auto it      = s_PickingData->PickingMap.find(id);
            if (it != s_PickingData->PickingMap.end())
                return it->second;
        }

        return {};
    }

    Ref<Texture2D> PickingManager::GetPickingTarget(uint32_t p_ID)
    {
        KTN_PROFILE_FUNCTION();

        if (!Engine::Get().GetSettings().MousePicking)
            return nullptr;

        auto it = s_PickingData->PickingTargets.find(p_ID);
        if (it != s_PickingData->PickingTargets.end())
            return it->second;

        return nullptr;
    }

} // namespace KTN
