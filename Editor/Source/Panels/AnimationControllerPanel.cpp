#include "AnimationControllerPanel.h"
#include "Editor.h"
#include "Koten/Asset/AssetManager.h"
#include "Koten/Asset/AnimationImporter.h"

// lib
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_node_editor_internal.h>
#include <imgui_bezier_math.h>



namespace KTN
{
    namespace
    {
        static ImVec2 Bezier(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, float t)
        {
            KTN_PROFILE_FUNCTION_LOW();

            float u   = 1.0f - t;
            float tt  = t * t;
            float uu  = u * u;
            float uuu = uu * u;
            float ttt = tt * t;

            return ImVec2(
                uuu * a.x + 3 * uu * t * b.x + 3 * u * tt * c.x + ttt * d.x,
                uuu * a.y + 3 * uu * t * b.y + 3 * u * tt * c.y + ttt * d.y
            );
        }

        static void DrawArrowFromLink(axnd::Detail::Link* p_Link)
        {
            KTN_PROFILE_FUNCTION_LOW();

            auto curve  = p_Link->GetCurve();

            float t     = 0.95f;

            ImVec2 pos  = Bezier(curve.P0, curve.P1, curve.P2, curve.P3, t);
            ImVec2 prev = Bezier(curve.P0, curve.P1, curve.P2, curve.P3, t - 0.02f);

            ImVec2 dir  = { pos.x - prev.x, pos.y - prev.y };
            float len   = sqrtf(dir.x * dir.x + dir.y * dir.y);
            if (len < 0.001f)
                return;

            dir.x      /= len;
            dir.y      /= len;

            ImVec2 perp = { -dir.y, dir.x };

            float size  = 8.0f;
            float width = size * 0.5f;

            ImVec2 tip  = pos;

            ImVec2 left = {
                pos.x - dir.x * size + perp.x * width,
                pos.y - dir.y * size + perp.y * width
            };

            ImVec2 right = {
                pos.x - dir.x * size - perp.x * width,
                pos.y - dir.y * size - perp.y * width
            };

            ImDrawList* drawList = ImGui::GetWindowDrawList();

            drawList->AddTriangleFilled(
                tip,
                left,
                right,
                p_Link->m_Color
            );
        }

    } // namespace

    void AnimationControllerPanel::OnImgui()
    {
        KTN_PROFILE_FUNCTION();

        if (!m_Controller) return;

        bool wasActive = m_Active;

        if (ImGui::Begin(m_Name.c_str(), &m_Active))
        {
            if (ImGui::BeginTable("AnimControllerLayout", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInner))
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
            m_Controller = nullptr;
            m_Nodes.clear();
            m_Pins.clear();
            m_Links.clear();
        }
    }

    void AnimationControllerPanel::Open()
    {
        KTN_PROFILE_FUNCTION();

        m_Active     = true;
        m_Controller = CreateRef<AnimationController>();
        CreateContext();
    }

    void AnimationControllerPanel::Open(const std::string& p_Path)
    {
        KTN_PROFILE_FUNCTION();

        m_Active = true;
        LoadPath(p_Path);
        CreateContext();
    }

    void AnimationControllerPanel::DrawLeftPanel()
    {
        KTN_PROFILE_FUNCTION();

        if (!m_Controller || !m_Controller->AnimationHandle) return;

        auto size = ImGui::GetContentRegionAvail();
        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
        bool opened = ImGui::BeginChild("LeftPanel", { size.x, size.y - (lineHeight * 1.5f) }, false);
        ImGui::PopStyleVar();

        if (opened)
        {
            auto anim = AssetManager::Get()->GetAsset<Animation>(m_Controller->AnimationHandle);

            if (!m_SelectedStates.empty())
            {
                ImGui::Text("States (%d)", (int)m_SelectedStates.size());

                if (m_SelectedStates.size() == 1)
                {
                    auto* state = m_SelectedStates[0];

                    UI::InputText("Name", state->Name, true, 0, 2.0f, true);
                }
                else
                {
                    ImGui::TextDisabled("Multiple selection");
                }
            }

            if (!m_SelectedTransitions.empty())
            {
                ImGui::Text("Transitions (%d)", (int)m_SelectedTransitions.size());

                if (m_SelectedTransitions.size() == 1)
                {
                    auto* t = m_SelectedTransitions[0];

                    ImGui::Checkbox("Has Exit Time", &t->HasExitTime);
                    if (t->HasExitTime)
                    {
                        ImGui::DragFloat("Exit Time", &t->ExitTime, 0.01f, 0.0f, 1.0f);
                        ImGui::DragFloat("Blend Time", &t->BlendTime, 0.01f, 0.0f, 1.0f);
                    }

                    DrawConditionsEditor(*t);
                }
                else
                {
                    ImGui::TextDisabled("Multiple selection");
                }
            }

            if (ImGui::CollapsingHeader("Parameters"))
            {
                if (ImGui::Button("Add Parameter"))
                {
                    AnimationController::Parameter param = {};
                    param.Name                           = "New Parameter " + std::to_string(m_Controller->Parameters.size());
                    param.ID                             = HashString(param.Name);
                    param.Type                           = ParameterType::Bool;
                    m_Controller->Parameters.emplace_back(param);
                }

                for (auto& param : m_Controller->Parameters)
                {
                    ImGui::PushID(param.Name.c_str());

                    // TODO: Maybe add a button to save the name change instead of saving on enter, to avoid accidental renames?
                    if (UI::InputText("Name", param.Name, true, ImGuiInputTextFlags_EnterReturnsTrue, 2.0f, true))
                    {
                        uint64_t newParamID = HashString(param.Name);

                        for (auto& state : m_Controller->States)
                        {
                            for (auto& transition : state.Transitions)
                            {
                                for (auto& cond : transition.Conditions)
                                {
                                    if (cond.ParameterID == param.ID)
                                        cond.ParameterID = newParamID;
                                }
                            }
                        }

                        param.ID = newParamID;
                    }
                    ImGui::Combo("Type", (int*)&param.Type, "Bool\0Float\0Int\0");

                    if (ImGui::Button("Remove"))
                    {
                        m_Controller->Parameters.erase(std::remove_if(m_Controller->Parameters.begin(), m_Controller->Parameters.end(),
                        [&](const AnimationController::Parameter& p)
                        {
                            return p.ID == param.ID;
                        }), m_Controller->Parameters.end());
                        ImGui::PopID();
                        break;
                    }
                    ImGui::Separator();
                    ImGui::PopID();
                }
            }

            if (ImGui::CollapsingHeader("Animations"))
            {
                for (const auto& clip : anim->Clips)
                {
                    ImGui::PushID(clip.ID);

                    ImGui::Selectable(clip.Name.c_str());

                    if (ImGui::BeginPopupContextItem("Options"))
                    {
                        auto it = std::find_if(m_Nodes.begin(), m_Nodes.end(), [&](const AnimNode& p_Value)
                        {
                            return p_Value.ClipID == clip.ID;
                        });

                        if (ImGui::MenuItem("Add") && it == m_Nodes.end())
                        {
                            AnimationState state = {};
                            state.Name           = clip.Name;
                            state.ClipID         = clip.ID;
                            state.ID             = HashString(state.Name);

                            AnimNode node        = {};
                            node.ID              = state.ID;
                            node.ClipID          = clip.ID;
                            node.Name            = clip.Name;

                            uint64_t in          = m_NextId++;
                            uint64_t out         = m_NextId++;

                            node.Inputs.push_back(in);
                            node.Outputs.push_back(out);

                            m_Pins.push_back({ .ID = in,  .NodeId = node.ID, .Kind = axnd::PinKind::Input });
                            m_Pins.push_back({ .ID = out, .NodeId = node.ID, .Kind = axnd::PinKind::Output });

                            m_Nodes.push_back(node);

                            m_Controller->States.push_back(state);

                            m_Controller->BuildStateMap();
                        }

                        ImGui::EndPopup();
                    }

                    ImGui::PopID();
                }
            }
        }

        ImGui::EndChild();

        if (ImGui::Button("Save", { 0.0f , lineHeight }))
        {
            m_Controller->BuildStateMap();

            FileSystem::CreateDirectories(Project::GetAssetFileSystemPath("Animations").string());
            auto& fpath = AssetManager::Get()->GetMetadata(m_Controller->AnimationHandle).FilePath;
            auto name   = FileSystem::GetStem(fpath);

            AnimationControllerImporter::Save(m_Controller, Project::GetAssetFileSystemPath("Animations/" + name + ".ktcontroller").string());
        }
    }

    void AnimationControllerPanel::DrawRightPanel()
    {
        KTN_PROFILE_FUNCTION();

        if (!m_Controller->AnimationHandle)
        {
            ImGui::BeginGroup();
            bool opened = ImGui::BeginChild("RightPanel", ImVec2(0, 0), false);
            if (opened)
            {
                ImVec2 size = ImGui::GetContentRegionAvail();

                ImGui::Dummy({ size.x * 0.3f, size.y * 0.3f });

                ImGui::SetCursorPosX((size.x - 120) * 0.5f);
                ImGui::Text("Drag and drop or");

                ImGui::Spacing();

                ImGui::SetCursorPosX((size.x - 120) * 0.5f);
                if (ImGui::Button("Select", ImVec2(120, 0)))
                {
                    std::string path = "";

                    if (FileDialog::Open({ {"AnimationController", "*.ktcontroller;*.ktanim"} }, "", path) == FileDialogResult::SUCCESS)
                    {
                        LoadPath(path);
                    }
                }
            }
            ImGui::EndChild();
            ImGui::EndGroup();

            if (ImGui::BeginDragDropTarget())
            {
                if (auto payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    const wchar_t* path = (const wchar_t*)payload->Data;
                    auto filepath = std::filesystem::path(path);

                    LoadPath(filepath.string());
                }

                ImGui::EndDragDropTarget();
            }
            return;
        }

        auto anim = AssetManager::Get()->GetAsset<Animation>(m_Controller->AnimationHandle);

        axnd::SetCurrentEditor(m_Context);
        axnd::Begin("RightPanel", ImVec2(0.0, 0.0f));

        UpdateSelecteds();

        if (ImGui::IsKeyPressed(ImGuiKey_Delete))
        {
            for (auto* t : m_SelectedTransitions)
            {
                auto it = std::find_if(m_Links.begin(), m_Links.end(),
                [&](const AnimLink& l)
                {
                    return l.TransitionID == t->ID;
                });

                if (it != m_Links.end())
                    RemoveTransition(it->ID);
            }

            for (auto* s : m_SelectedStates)
            {
                if (s->ID == m_Controller->EntryState.ID)
                    continue;

                RemoveState(s->ID);
            }

            m_SelectedStates.clear();
            m_SelectedTransitions.clear();
            axnd::ClearSelection();
        }

        for (auto& node : m_Nodes)
        {
            if (node.IsEntry)
            {
                axnd::PushStyleColor(axnd::StyleColor_NodeBg, ImVec4(1.0f, 0.801f, 0.398f, 1.0f));
                axnd::PushStyleColor(axnd::StyleColor_NodeBorder, ImVec4(1.0f, 0.649f, 0.1f, 1.0f));
            }

            axnd::BeginNode(node.ID);

            if (node.IsEntry)
                axnd::PopStyleColor(2);

            ImGui::Text("%s", node.Name.c_str());

            // Inputs
            for (auto pinID : node.Inputs)
            {
                axnd::BeginPin(pinID, axnd::PinKind::Input);
                ImGui::Dummy(ImVec2(10, 10));
                axnd::EndPin();
            }

            ImGui::SameLine();

            // Outputs
            for (auto pinID : node.Outputs)
            {
                axnd::BeginPin(pinID, axnd::PinKind::Output);
                ImGui::Dummy(ImVec2(10, 10));
                axnd::EndPin();
            }

            axnd::EndNode();
        }

        if (axnd::BeginCreate())
        {
            axnd::PinId a, b;

            if (axnd::QueryNewLink(&a, &b))
            {
                if (a && b)
                {
                    auto* pinA = FindPin((uint64_t)a.Get());
                    auto* pinB = FindPin((uint64_t)b.Get());

                    if (pinA && pinB)
                    {
                        if (pinA->Kind == pinB->Kind)
                            axnd::RejectNewItem();
                        else
                        {
                            AnimPin* input            = pinA->Kind == axnd::PinKind::Input ? pinA : pinB;
                            AnimPin* output           = pinA->Kind == axnd::PinKind::Output ? pinA : pinB;

                            if (axnd::AcceptNewItem())
                            {
                                AnimationState* state = nullptr;
                                if (m_Controller->EntryState.ID == output->NodeId)
                                    state = &m_Controller->EntryState;
                                else
                                    state = m_Controller->Get(output->NodeId);

                                if (state)
                                {
                                    AnimationTransition transition = {};
                                    transition.ID                  = UUID(); // gen UUID for transition
                                    transition.ToState             = input->NodeId;
                                    transition.ToStateIndex        = m_Controller->StateMap[input->NodeId];
                                    HashCombine( state->ID, input->NodeId, output->NodeId);

                                    state->Transitions.push_back(transition);
                                    auto& created = state->Transitions.back();

                                    m_Links.push_back({
                                        m_NextId++,
                                        output->ID,
                                        input->ID,
                                        transition.ID
                                    });
                                }
                            }
                        }
                    }

                }
            }
        }
        axnd::EndCreate();

        auto context = reinterpret_cast<axnd::Detail::EditorContext*>(m_Context);

        for (auto& link : m_Links)
        {
            auto dLink = context->GetLink(link.ID);

            axnd::Link(link.ID, link.StartPin, link.EndPin);

            if (dLink)
                DrawArrowFromLink(dLink);
        }

        axnd::End();
        axnd::SetCurrentEditor(nullptr);
    }

    void AnimationControllerPanel::LoadPath(const std::string& p_Path)
    {
        KTN_PROFILE_FUNCTION();

        auto ext = FileSystem::GetExtension(p_Path);
        if (ext == ".ktanim")
        {
            if (!m_Controller) m_Controller = CreateRef<AnimationController>();
            m_Controller->AnimationHandle = AssetManager::Get()->ImportAsset(AssetType::Animation, p_Path);
        }
        else if (ext == ".ktcontroller")
        {
            auto controller = AssetManager::Get()->ImportAsset(AssetType::AnimationController, p_Path);
            m_Controller = AssetManager::Get()->GetAsset<AnimationController>(controller);

            RebuildGraphFromController();
        }
    }

    void AnimationControllerPanel::CreateContext()
    {
        KTN_PROFILE_FUNCTION();

        if (m_Context) DestroyContext();

        axnd::Config config;
        config.SettingsFile = nullptr;

        m_Context = axnd::CreateEditor(&config);

        CreateEntryNode();
    }

    void AnimationControllerPanel::DestroyContext()
    {
        KTN_PROFILE_FUNCTION();

        if (m_Context)
        {
            axnd::DestroyEditor(m_Context);
            m_Context = nullptr;
        }
    }

    void AnimationControllerPanel::CreateEntryNode()
    {
        KTN_PROFILE_FUNCTION();

        AnimNode node   = {};
        node.ID         = m_Controller->EntryState.ID;
        node.Name       = m_Controller->EntryState.Name;
        node.IsEntry    = true;

        uint64_t outPin = m_NextId++;
        node.Outputs.push_back(outPin);

        m_Pins.push_back({ outPin, node.ID, axnd::PinKind::Output });

        m_Nodes.push_back(node);

        UpdateInitialNodePosition(node.ID);
    }

    void AnimationControllerPanel::RemoveState(uint64_t p_StateID)
    {
        KTN_PROFILE_FUNCTION();

        if (p_StateID == m_Controller->EntryState.ID)
            return;

        auto& states = m_Controller->States;

        states.erase(std::remove_if(states.begin(), states.end(),
        [&](const AnimationState& s)
        {
            return s.ID == p_StateID;
        }), states.end());

        for (auto& state : states)
        {
            state.Transitions.erase(std::remove_if(state.Transitions.begin(), state.Transitions.end(),
            [&](const AnimationTransition& t)
            {
                return t.ToState == p_StateID;
            }), state.Transitions.end());
        }

        m_Nodes.erase(std::remove_if(m_Nodes.begin(), m_Nodes.end(),
        [&](const AnimNode& n)
        {
            return n.ID == p_StateID;
        }), m_Nodes.end());

        m_Links.erase(std::remove_if(m_Links.begin(), m_Links.end(),
        [&](const AnimLink& l)
        {
            auto* s = FindPin(l.StartPin);
            auto* e = FindPin(l.EndPin);
            if (!s || !e) return false;

            return s->NodeId == p_StateID || e->NodeId == p_StateID;
        }), m_Links.end());

        m_Controller->BuildStateMap();
    }

    void AnimationControllerPanel::RemoveTransition(uint64_t p_LinkID)
    {
        KTN_PROFILE_FUNCTION();

        auto it = std::find_if(m_Links.begin(), m_Links.end(),
        [&](const AnimLink& l)
        {
            return l.ID == p_LinkID;
        });

        if (it == m_Links.end())
            return;

        auto* transition = m_Controller->FindTransition(it->TransitionID);

        auto removeLink = [&](AnimationState& state)
        {
            auto& list = state.Transitions;

            auto tIt = std::find_if(list.begin(), list.end(),
            [&](AnimationTransition& t)
            {
                return t.ID == transition->ID;
            });

            if (tIt != list.end())
            {
                list.erase(tIt);
                return true;
            }

            return false;
        };

        auto startPin = FindPin(it->StartPin);
        if (!startPin)
        {
            m_Links.erase(it);
            return;
        }

        if (m_Controller->EntryState.ID == startPin->NodeId)
            removeLink(m_Controller->EntryState);
        else
        {
            for (auto& state : m_Controller->States)
            {
                if (removeLink(state))
                    break;
            }
        }

        m_Links.erase(it);
    }

    void AnimationControllerPanel::DrawConditionsEditor(AnimationTransition& p_Transition)
    {
        KTN_PROFILE_FUNCTION();

        if (ImGui::CollapsingHeader("Conditions"))
        {
            for (int i = 0; i < p_Transition.Conditions.size(); i++)
            {
                auto& cond = p_Transition.Conditions[i];

                ImGui::PushID(i);

                std::string paramName = "Select Parameter";
                if (cond.ParameterID)
                {
                    auto* param       = m_Controller->GetParameter(cond.ParameterID);
                    if (param)
                        paramName     = param->Name;
                }

                if (ImGui::Button(paramName.c_str()))
                {
                    ImGui::OpenPopup("ParameterSelector");
                }

                ImGui::Combo("Compare Operator", (int*)&cond.Operator, "Select an Operator\0==\0!=\0>\0<\0>=\0<=\0");

                switch (cond.Type)
                {
                    case ParameterType::Bool:
                        ImGui::Checkbox("Compare Value", &cond.CompareValue.Bool);
                        break;

                    case ParameterType::Float:
                        ImGui::DragFloat("Compare Value", &cond.CompareValue.Float);
                        break;

                    case ParameterType::Int:
                        ImGui::DragInt("Compare Value", &cond.CompareValue.Int);
                        break;
                }

                if (ImGui::BeginPopup("ParameterSelector"))
                {
                    KTN_PROFILE_FUNCTION();

                    static char buffer[128] = {};

                    ImGui::Text(ICON_MDI_SEARCH_WEB);
                    ImGui::SameLine();
                    ImGui::InputText("##Search", buffer, sizeof(buffer), ImGuiInputTextFlags_AutoSelectAll);

                    auto size = ImGui::GetContentRegionAvail();
                    ImGui::SetNextWindowSizeConstraints({ size.x, 50.0f }, { size.x, 150.0f });
                    ImGui::BeginChild("##ClassesList", { 0, 0 }, true);
                    {
                        for (const auto& param : m_Controller->Parameters)
                        {
                            auto hash = HashString(param.Name);

                            if (buffer[0] != '\0')
                            {
                                std::string nameLower = param.Name;
                                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

                                char bufferLower[128];
                                Strncpy(bufferLower, buffer, sizeof(bufferLower));
                                std::transform(bufferLower, bufferLower + sizeof(bufferLower), bufferLower, ::tolower);

                                if (nameLower.find(bufferLower) == std::string::npos)
                                    continue; // Skip if the name does not match the search
                            }

                            if (ImGui::Selectable(param.Name.c_str(), cond.ParameterID == hash))
                            {
                                cond.ParameterID = hash;
                                cond.Type        = param.Type;
                                ImGui::CloseCurrentPopup();
                            }
                        }
                    }
                    ImGui::EndChild();

                    ImGui::EndPopup();
                }

                if (ImGui::Button("Remove"))
                {
                    p_Transition.Conditions.erase(p_Transition.Conditions.begin() + i);
                    ImGui::PopID();
                    break;
                }

                ImGui::Separator();
                ImGui::PopID();
            }

            if (ImGui::Button("Add Condition"))
            {
                p_Transition.Conditions.emplace_back();
            }
        }
    }

    AnimPin* AnimationControllerPanel::FindPin(uint64_t p_ID)
    {
        KTN_PROFILE_FUNCTION();

        for (auto& p : m_Pins)
            if (p.ID == p_ID)
                return &p;

        return nullptr;
    }

    void AnimationControllerPanel::UpdateSelecteds()
    {
        KTN_PROFILE_FUNCTION();

        m_SelectedStates.clear();
        m_SelectedTransitions.clear();

        int nodeCount = axnd::GetSelectedNodes(nullptr, 0);
        int linkCount = axnd::GetSelectedLinks(nullptr, 0);

        if (nodeCount == 0 && linkCount == 0)
            return;

        std::vector<axnd::NodeId> nodes(nodeCount);
        int fetchedNodes = axnd::GetSelectedNodes(nodes.data(), nodeCount);

        for (int i = 0; i < fetchedNodes; i++)
        {
            uint64_t id = (uint64_t)nodes[i];

            if (id == m_Controller->EntryState.ID)
                continue;

            auto* state = m_Controller->Get(id);
            if (state)
                m_SelectedStates.push_back(state);
        }

        std::vector<axnd::LinkId> links(linkCount);
        int fetchedLinks = axnd::GetSelectedLinks(links.data(), linkCount);

        for (int i = 0; i < fetchedLinks; i++)
        {
            uint64_t id = (uint64_t)links[i];

            for (auto& link : m_Links)
            {
                if (link.ID == id && link.TransitionID != 0)
                {
                    m_SelectedTransitions.push_back(m_Controller->FindTransition(link.TransitionID));
                    break;
                }
            }
        }
    }

    void AnimationControllerPanel::RebuildGraphFromController()
    {
        KTN_PROFILE_FUNCTION();

        m_Nodes.clear();
        m_Pins.clear();
        m_Links.clear();

        m_NextId = 1;
        m_NextX  = 0.0f;
        m_NextY  = 0.0f;

        if (!m_Controller)
            return;

        CreateEntryNode();

        for (auto& state : m_Controller->States)
        {
            AnimNode node = {};
            node.ID       = state.ID;
            node.Name     = state.Name;
            node.ClipID   = state.ClipID;

            uint64_t in   = m_NextId++;
            uint64_t out  = m_NextId++;

            node.Inputs.push_back(in);
            node.Outputs.push_back(out);

            m_Pins.push_back({ in,  node.ID, axnd::PinKind::Input });
            m_Pins.push_back({ out, node.ID, axnd::PinKind::Output });

            m_Nodes.push_back(node);

            UpdateInitialNodePosition(node.ID);
        }

        auto findInputPin = [&](uint64_t nodeID) -> AnimPin*
        {
            for (auto& pin : m_Pins)
                if (pin.NodeId == nodeID && pin.Kind == axnd::PinKind::Input)
                    return &pin;

            return nullptr;
        };

        auto findOutputPin = [&](uint64_t nodeID) -> AnimPin*
        {
            for (auto& pin : m_Pins)
                if (pin.NodeId == nodeID && pin.Kind == axnd::PinKind::Output)
                    return &pin;

            return nullptr;
        };

        {
            auto* entryOut = findOutputPin(m_Controller->EntryState.ID);

            for (auto& t : m_Controller->EntryState.Transitions)
            {
                auto* targetIn = findInputPin(t.ToState);

                if (!entryOut || !targetIn)
                    continue;

                m_Links.push_back({
                    m_NextId++,
                    entryOut->ID,
                    targetIn->ID,
                    t.ID
                });
            }
        }

        for (auto& state : m_Controller->States)
        {
            auto* out = findOutputPin(state.ID);

            for (auto& t : state.Transitions)
            {
                auto* in = findInputPin(t.ToState);

                if (!out || !in)
                    continue;

                m_Links.push_back({
                    m_NextId++,
                    out->ID,
                    in->ID,
                    t.ID
                });
            }
        }
    }

    void AnimationControllerPanel::UpdateInitialNodePosition(uint64_t p_NodeID)
    {
        KTN_PROFILE_FUNCTION_LOW();

        auto context = reinterpret_cast<axnd::Detail::EditorContext*>(m_Context);
        context->SetNodePosition(p_NodeID, ImVec2(m_NextX, m_NextY));

        m_NextY += 100.0f;
        if (m_NextY > 500.0f)
        {
            m_NextY = 0.0f;
            m_NextX += 250.0f;
        }
    }

} // namespace KTN
