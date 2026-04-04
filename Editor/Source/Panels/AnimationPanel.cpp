#include "AnimationPanel.h"
#include "Editor.h"
#include "Koten/Asset/AssetManager.h"
#include "Koten/Asset/TextureAtlasImporter.h"

// lib
#include <imgui.h>
#include <imgui_internal.h>



namespace KTN
{
    namespace
    {
        static void NormalizeFrames(AnimationClip& p_Clip)
        {
            KTN_PROFILE_FUNCTION_LOW();

            if (p_Clip.Frames.empty())
                return;

            std::sort(p_Clip.Frames.begin(), p_Clip.Frames.end(),
            [](const AnimationFrame& p_A, const AnimationFrame& p_B)
            {
                return p_A.Time < p_B.Time;
            });

            const float epsilon = 0.001f;

            for (size_t i = 1; i < p_Clip.Frames.size(); i++)
            {
                if (p_Clip.Frames[i].Time <= p_Clip.Frames[i - 1].Time)
                    p_Clip.Frames[i].Time = p_Clip.Frames[i - 1].Time + epsilon;
            }

            for (auto& f : p_Clip.Frames)
            {
                f.Time = std::clamp(f.Time, 0.0f, p_Clip.TotalDuration);
            }
        }

        static float Snap(float p_V)
        {
            KTN_PROFILE_FUNCTION_LOW();

            return floorf(p_V / 0.05f + 0.5f) * 0.05f;
        }

        static bool HasFrameAtTime(const AnimationClip& p_Clip, float p_Time, int p_IgnoreIndex)
        {
            KTN_PROFILE_FUNCTION_LOW();

            for (int i = 0; i < (int)p_Clip.Frames.size(); i++)
            {
                if (i == p_IgnoreIndex) continue;

                if (fabs(p_Clip.Frames[i].Time - p_Time) < 0.001f)
                    return true;
            }
            return false;
        }

        static float ResolveFrameCollision(AnimationClip& p_Clip, float p_Time, float p_TotalDuration, int p_Index)
        {
            KTN_PROFILE_FUNCTION_LOW();

            float step = 0.05f;

            float t = p_Time;

            while (true)
            {
                if (!HasFrameAtTime(p_Clip, t, p_Index))
                    return t;

                if (t - step >= 0.0f)
                {
                    t -= step;
                    t = Snap(t);
                }
                else
                {
                    t += step;
                    t = Snap(t);

                    if (t > p_TotalDuration)
                        t = p_TotalDuration;
                }
            }
        }

    } // namespace

    void AnimationPanel::OnImgui()
    {
        KTN_PROFILE_FUNCTION();

        if (!m_Anim) return;

        bool wasActive = m_Active;

        if (ImGui::Begin(m_Name.c_str(), &m_Active))
        {
            if (ImGui::BeginTable("AnimLayout", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInner))
            {
                ImGui::TableNextColumn();
                DrawLeftPanel();

                ImGui::TableNextColumn();
                DrawRightPanel();

                ImGui::EndTable();
            }
        }
        ImGui::End();

        if (wasActive && !m_Active)
        {
            m_Anim           = nullptr;
            m_SelectedClipID = 0;
            m_AnimationName  = "New Animation";
        }
    }

    void AnimationPanel::Open()
    {
        KTN_PROFILE_FUNCTION();

        m_Active         = true;
        m_SelectedClipID = 0;
        m_AnimationName  = "New Animation";
        m_Anim           = CreateRef<Animation>();
    }

    void AnimationPanel::Open(const std::string& p_Path)
    {
        KTN_PROFILE_FUNCTION();

        m_Active         = true;
        m_SelectedClipID = 0;
        m_AnimationName  = "New Animation";
        LoadPath(p_Path);
    }

    void AnimationPanel::DrawLeftPanel()
    {
        KTN_PROFILE_FUNCTION();

        auto size        = ImGui::GetContentRegionAvail();
        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
        bool opened      = ImGui::BeginChild("LeftPanel", { size.x, size.y - (lineHeight * 1.5f) }, false);
        ImGui::PopStyleVar();

        if (opened)
        {
            if (ImGui::CollapsingHeader("Settings"))
            {
                UI::InputText("Name", m_AnimationName, true, 0, 2.0f, true);

                if (m_SelectedClipID)
                {
                    auto clip                   = m_Anim->Get(m_SelectedClipID);
                    UI::InputText("Clip Name", clip->Name, true, 0, 2.0f, true);

                    int currentItem             = (int)clip->Mode;
                    static const char* items[]  = { "Loop", "PingPong", "Once" };
                    static const int itemsCount = IM_ARRAYSIZE(items);

                    if (UI::Combo("Mode", items[currentItem], items, itemsCount, &currentItem, -1))
                    {
                        clip->Mode = (AnimationMode)currentItem;
                    }

                    ImGui::Text("Speed"); ImGui::SameLine();
                    ImGui::DragFloat("##Speed", &clip->Speed, 0.01f, 0.01f, 10.0f);
                    ImGui::InputFloat("##Duration", &clip->TotalDuration, 0.05f, 0.1f, "%.2f");
                }
            }

            if (ImGui::CollapsingHeader("Animations") && m_Anim->TextureAtlas)
            {
                if (ImGui::Button("Add Animation", { 0.0f, lineHeight }))
                {
                    AnimationClip clip = {};
                    clip.Name = "Animation " + std::to_string(m_Anim->GetCount());
                    clip.Mode = AnimationMode::Loop;
                    m_Anim->Clips.push_back(clip);

                    m_Anim->BuildClipMap();
                }
    
                for (size_t i = 0; i < m_Anim->Clips.size(); i++)
                {
                    auto& clip = m_Anim->Clips[i];
    
                    ImGui::PushID(clip.ID);
                    if (ImGui::Selectable(clip.Name.c_str(), m_SelectedClipID == clip.ID))
                    {
                        m_SelectedClipID = clip.ID;
                    }
                    ImGui::PopID();
                }
            }

            if (ImGui::CollapsingHeader("Regions"))
            {
                if (m_Anim->TextureAtlas)
                {
                    float panelWidth     = ImGui::GetContentRegionAvail().x;
                    float itemWidth      = 80.0f;
                    int columns          = std::max(1, (int)(panelWidth / itemWidth));

                    ImGui::Columns(columns, "RegionGrid", false);

                    auto atlas           = AssetManager::Get()->GetAsset<TextureAtlas>(m_Anim->TextureAtlas);

                    for (const auto& region : atlas->Regions)
                    {
                        ImGui::BeginGroup();

                        auto texture     = AssetManager::Get()->GetAsset<Texture2D>(atlas->Texture);
                        ImGui::PushID(region.ID);

                        float buttonSize = itemWidth - 20.0f;

                        UI::ImageButton(texture, { buttonSize, buttonSize }, { region.UV.x, region.UV.y }, { region.UV.z, region.UV.w });

                        if (ImGui::BeginDragDropSource())
                        {
                            ImGui::SetDragDropPayload("ANIM_REGION", &region.ID, sizeof(uint64_t));
                            ImGui::EndDragDropSource();
                        }

                        float textWidth  = ImGui::CalcTextSize(region.Name.c_str()).x;
                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (buttonSize - textWidth) * 0.5f);
                        ImGui::Text(region.Name.c_str());


                        ImGui::PopID();
                        ImGui::EndGroup();

                        ImGui::NextColumn();
                    }

                    ImGui::Columns(1);
                }
            }

        }

        ImGui::EndChild();

        if (ImGui::Button("Save", { 0.0f , lineHeight }))
        {
            m_Anim->BuildClipMap();

            for (auto& clip : m_Anim->Clips)
                NormalizeFrames(clip);

            FileSystem::CreateDirectories(Project::GetAssetFileSystemPath("Animations").string());
            AnimationImporter::Save(m_Anim, Project::GetAssetFileSystemPath("Animations/" + m_AnimationName + ".ktanim").string());
        }
    }

    void AnimationPanel::DrawRightPanel()
    {
        KTN_PROFILE_FUNCTION();

        ImGui::BeginGroup();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
        bool opened = ImGui::BeginChild("RightPanel", ImVec2(0, 0), false);
        ImGui::PopStyleVar();

        if (opened)
        {
            if (!m_Anim->TextureAtlas)
            {
                ImVec2 size              = ImGui::GetContentRegionAvail();

                ImGui::Dummy({ size.x * 0.3f, size.y * 0.3f });

                ImGui::SetCursorPosX((size.x - 120) * 0.5f);
                ImGui::Text("Drag and drop or");

                ImGui::Spacing();

                ImGui::SetCursorPosX((size.x - 120) * 0.5f);
                if (ImGui::Button("Select", ImVec2(120, 0)))
                { 
                    std::string path     = "";

                    if (FileDialog::Open({ {"Animation", "*.ktatlas;*.ktanim"} }, "", path) == FileDialogResult::SUCCESS)
                    {
                        m_SelectedClipID = 0;
                        m_AnimationName  = "New Animation";
                        LoadPath(path);
                    }
                }
            }
            else if (!m_SelectedClipID)
            {
                ImVec2 size = ImGui::GetContentRegionAvail();
                ImGui::Dummy({ size.x * 0.3f, size.y * 0.3f });
                ImGui::SetCursorPosX((size.x - 120) * 0.5f);

                ImGui::Text("Select a clip");
            }
            else
            {
                auto* clip               = m_Anim->Get(m_SelectedClipID);

                auto atlas               = AssetManager::Get()->GetAsset<TextureAtlas>(m_Anim->TextureAtlas);
                auto texture             = AssetManager::Get()->GetAsset<Texture2D>(atlas->Texture);

                float totalDuration      = std::max(clip->TotalDuration, 0.001f);

                static float currentTime = 0.0f;
                static bool  isPlaying   = true;
                static bool  forward     = true;

                float buttonSize         = ImGui::GetFrameHeight();
                float spacing            = ImGui::GetStyle().ItemSpacing.x;
                float totalWidth         = buttonSize * 3 + spacing * 2;

                float avail              = ImGui::GetContentRegionAvail().x;
                float offset             = (avail - totalWidth) * 0.5f;

                if (offset > 0)
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

                if (ImGui::Button(isPlaying ? ICON_MDI_PAUSE : ICON_MDI_PLAY))
                {
                    isPlaying            = !isPlaying;
                }

                ImGui::SameLine();

                if (ImGui::Button(ICON_MDI_STOP))
                {
                    isPlaying   = false;
                    currentTime = 0.0f;
                    forward     = true;
                }

                ImGui::SameLine();

                auto stepIcon                = clip->Mode == AnimationMode::PingPong ? forward ? ICON_MDI_STEP_FORWARD : ICON_MDI_STEP_BACKWARD : ICON_MDI_STEP_FORWARD;
                if (ImGui::Button(stepIcon))
                {
                    if (!clip->Frames.empty())
                    {
                        const float epsilon  = 0.0001f;

                        int currentIndex     = 0;
                        for (int i = 0; i < (int)clip->Frames.size(); i++)
                        {
                            if (clip->Frames[i].Time <= currentTime + epsilon)
                                currentIndex = i;
                            else
                                break;
                        }

                        int lastIndex    = (int)clip->Frames.size() - 1;

                        switch (clip->Mode)
                        {
                            case AnimationMode::Loop:
                            {
                                currentIndex = (currentIndex + 1) % clip->Frames.size();
                                currentTime  = clip->Frames[currentIndex].Time;
                                break;
                            }

                            case AnimationMode::Once:
                            {
                                if (currentIndex < lastIndex)
                                {
                                    currentIndex++;
                                    currentTime = clip->Frames[currentIndex].Time;
                                }
                                else
                                {
                                    currentTime = clip->Frames[lastIndex].Time;
                                }
                                break;
                            }

                            case AnimationMode::PingPong:
                            {
                                if (forward)
                                {
                                    if (currentIndex < lastIndex)
                                        currentIndex++;
                                    else
                                    {
                                        forward = false;
                                        currentIndex--;
                                    }
                                }
                                else
                                {
                                    if (currentIndex > 0)
                                        currentIndex--;
                                    else
                                    {
                                        forward = true;
                                        currentIndex++;
                                    }
                                }

                                currentTime = clip->Frames[currentIndex].Time;
                                break;
                            }
                        }
                    }
                }

                ImGui::BeginChild("AnimationPreview", ImVec2(0, 200), true);

                if (!clip->Frames.empty())
                {
                    if (isPlaying)
                    {
                        float dt = ImGui::GetIO().DeltaTime * clip->Speed;

                        switch (clip->Mode)
                        {
                            case AnimationMode::Loop:
                            {
                                currentTime += dt;

                                if (currentTime > totalDuration)
                                    currentTime = fmod(currentTime, totalDuration);
                                break;
                            }

                            case AnimationMode::Once:
                            {
                                currentTime += dt;

                                if (currentTime > totalDuration)
                                    currentTime = totalDuration;
                                break;
                            }

                            case AnimationMode::PingPong:
                            {
                                if (forward)
                                {
                                    currentTime += dt;

                                    if (currentTime >= totalDuration)
                                    {
                                        currentTime = totalDuration;
                                        forward = false;
                                    }
                                }
                                else
                                {
                                    currentTime -= dt;

                                    if (currentTime <= 0.0f)
                                    {
                                        currentTime = 0.0f;
                                        forward = true;
                                    }
                                }
                                break;
                            }
                        }
                    }

                    const AnimationFrame* current = &clip->Frames.front();

                    for (size_t i = 0; i < clip->Frames.size(); i++)
                    {
                        if (clip->Frames[i].Time <= currentTime)
                            current = &clip->Frames[i];
                        else
                            break;
                    }

                    auto* region            = atlas->GetRegion(current->AtlasRegionID);
                    if (region)
                    {
                        ImVec2 size         = ImGui::GetContentRegionAvail();
                        ImVec2 gridSize     = { region->GridSize.x, region->GridSize.y };

                        float aspectRatio   = gridSize.x / gridSize.y;
                        ImVec2 imageSize    = size;

                        if (aspectRatio > 1.0f)
                        {
                            imageSize.x     = size.x;
                            imageSize.y     = size.x / aspectRatio;

                            if (imageSize.y > size.y)
                            {
                                imageSize.y = size.y;
                                imageSize.x = size.y * aspectRatio;
                            }
                        }
                        else
                        {
                            imageSize.y     = size.y;
                            imageSize.x     = size.y * aspectRatio;

                            if (imageSize.x > size.x)
                            {
                                imageSize.x = size.x;
                                imageSize.y = size.x / aspectRatio;
                            }
                        }

                        float posX          = (size.x - imageSize.x) * 0.5f;
                        float posY          = (size.y - imageSize.y) * 0.5f;

                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + posX);
                        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + posY);

                        UI::Image(texture, imageSize,
                            { region->UV.x, region->UV.y },
                            { region->UV.z, region->UV.w });
                    }
                }

                ImGui::EndChild();

                ImGui::BeginGroup();
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 1));
                ImGui::BeginChild("Timeline", ImVec2(0, 0), true);
                ImGui::PopStyleColor();

                float width          = ImGui::GetContentRegionAvail().x;
                float height         = ImGui::GetContentRegionAvail().y;

                ImVec2 timelineMin   = ImGui::GetWindowPos();
                ImVec2 timelineMax   = { timelineMin.x + width, timelineMin.y + height };

                bool mouseInTimeline =
                    ImGui::GetIO().MousePos.x >= timelineMin.x &&
                    ImGui::GetIO().MousePos.x <= timelineMax.x &&
                    ImGui::GetIO().MousePos.y >= timelineMin.y &&
                    ImGui::GetIO().MousePos.y <= timelineMax.y;

                ImDrawList* draw = ImGui::GetWindowDrawList();
                ImVec2 origin    = ImGui::GetCursorScreenPos();
                float centerY    = origin.y + height * 0.5f;

                float minorStep  = 0.05f;
                int steps        = (int)(totalDuration / minorStep) + 1;

                for (int i = 0; i <= steps; i++)
                {
                    float t          = i * minorStep;
                    if (t > totalDuration) break;

                    float norm       = t / totalDuration;
                    float x          = origin.x + norm * width;

                    bool major       = (i % 20 == 0);   // 1s
                    bool mid         = (i % 2 == 0);    // 0.1s

                    float lineHeight = major ? 20.0f : mid ? 12.0f : 6.0f;

                    ImU32 color      = major ? IM_COL32(200, 200, 200, 255) : mid ? IM_COL32(160, 160, 160, 180) : IM_COL32(120, 120, 120, 80);

                    draw->AddLine(
                        { x, centerY - lineHeight },
                        { x, centerY + lineHeight },
                        color
                    );

                    if (major)
                    {
                        char buf[16];
                        sprintf(buf, "%.0fs", t);
                        draw->AddText({ x + 2, centerY + 22 }, IM_COL32_WHITE, buf);
                    }
                }

                draw->AddLine(
                    { origin.x, centerY },
                    { origin.x + width, centerY },
                    IM_COL32(255, 255, 255, 255),
                    2.0f
                );

                static int selectedFrame = -1;
                bool clickedOnFrame      = false;

                for (size_t i = 0; i < clip->Frames.size(); i++)
                {
                    auto& frame          = clip->Frames[i];
                    float norm           = frame.Time / totalDuration;
                    float x              = origin.x + norm * width;

                    ImVec2 p[4] = {
                        { x, centerY - 6 },
                        { x + 6, centerY },
                        { x, centerY + 6 },
                        { x - 6, centerY }
                    };

                    ImGui::SetCursorScreenPos({ x - 6, centerY - 6 });
                    ImGui::InvisibleButton(("frame" + std::to_string(i)).c_str(), { 12, 12 });

                    if (ImGui::BeginPopupContextItem())
                    {
                        if (ImGui::MenuItem("Delete"))
                        {
                            clip->Frames.erase(clip->Frames.begin() + i);

                            if (selectedFrame == (int)i)
                                selectedFrame = -1;
                            else if (selectedFrame > (int)i)
                                selectedFrame--;

                            ImGui::EndPopup();
                            break;
                        }

                        ImGui::EndPopup();
                    }

                    bool hovered       = ImGui::IsItemHovered();
                    bool active        = ImGui::IsItemActive();
                    bool clicked       = ImGui::IsItemClicked();

                    if (clicked)
                    {
                        selectedFrame  = (int)i;
                        clickedOnFrame = true;
                    }

                    ImU32 color        = (selectedFrame == i) ? IM_COL32(100, 255, 100, 255) :
                                          hovered ? IM_COL32(255, 200, 100, 255) : IM_COL32(255, 255, 255, 255);

                    draw->AddConvexPolyFilled(p, 4, color);
                }

                static bool isDraggingFrame = false;

                if (selectedFrame != -1)
                {
                    if (ImGui::IsKeyPressed(ImGuiKey_Delete))
                    {
                        clip->Frames.erase(clip->Frames.begin() + selectedFrame);
                        selectedFrame = -1;
                    }

                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !mouseInTimeline)
                    {
                        selectedFrame = -1;
                    }

                    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && mouseInTimeline && !clickedOnFrame)
                    {
                        isDraggingFrame = true;

                        ImVec2 mousePos = ImGui::GetIO().MousePos;

                        float norm      = (mousePos.x - origin.x) / width;
                        norm            = std::clamp(norm, 0.0f, 1.0f);
                        float rawTime   = norm * totalDuration;

                        // snap
                        float step      = 0.05f;
                        float snapped   = std::round(rawTime / step) * step;

                        auto& frame     = clip->Frames[selectedFrame];
                        frame.Time      = snapped;
                    }

                    if (isDraggingFrame && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
                    {
                        auto& frame     = clip->Frames[selectedFrame];
                        frame.Time      = ResolveFrameCollision(*clip, frame.Time, totalDuration, selectedFrame);

                        std::sort(clip->Frames.begin(), clip->Frames.end(),
                        [](const AnimationFrame& a, const AnimationFrame& b)
                        {
                            return a.Time < b.Time;
                        });

                        isDraggingFrame = false;
                        selectedFrame   = -1;
                    }
                }


                ImGui::EndChild();
                ImGui::EndGroup();

                if (ImGui::BeginDragDropTarget())
                {
                    if (auto payload        = ImGui::AcceptDragDropPayload("ANIM_REGION"))
                    {
                        uint64_t regionID   = *(uint64_t*)payload->Data;

                        float mouseX        = ImGui::GetIO().MousePos.x;
                        float norm          = (mouseX - origin.x) / width;
                        norm                = std::clamp(norm, 0.0f, 1.0f);
                        float rawTime       = norm * totalDuration;
                        float step          = 0.05f;
                        float snapped       = std::round(rawTime / step) * step;

                        AnimationFrame frame;
                        frame.AtlasRegionID = regionID;
                        frame.Time          = snapped;
                        frame.Time          = ResolveFrameCollision(*clip, frame.Time, totalDuration, -1);

                        clip->Frames.push_back(frame);

                        std::sort(clip->Frames.begin(), clip->Frames.end(),
                        [](const AnimationFrame& a, const AnimationFrame& b)
                        {
                            return a.Time < b.Time;
                        });
                    }

                    ImGui::EndDragDropTarget();
                }
            }
        }
        ImGui::EndChild();
        ImGui::EndGroup();

        if (!m_Anim->TextureAtlas && ImGui::BeginDragDropTarget())
        {
            if (auto payload        = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                const wchar_t* path = (const wchar_t*)payload->Data;
                auto filepath       = std::filesystem::path(path);

                LoadPath(filepath.string());
            }

            ImGui::EndDragDropTarget();
        }
    }

    void AnimationPanel::LoadPath(const std::string& p_Path)
    {
        KTN_PROFILE_FUNCTION();

        auto ext                 = FileSystem::GetExtension(p_Path);
        if (ext == ".ktanim")
        {
            auto anim            = AssetManager::Get()->ImportAsset(AssetType::Animation, p_Path);
            m_Anim               = AssetManager::Get()->GetAsset<Animation>(anim);
            m_AnimationName      = FileSystem::GetName(p_Path);
        }
        else if (ext == ".ktatlas")
        {
            if (!m_Anim) m_Anim  = CreateRef<Animation>();
            m_Anim->TextureAtlas = AssetManager::Get()->ImportAsset(AssetType::TextureAtlas, p_Path);
        }
    }

} // namespace KTN
