#include "InspectorPanel.h"
#include "Editor.h"

// lib
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>



namespace KTN
{
	namespace
	{
		#define ALL_VIEW_COMPONENTS TransformComponent, CameraComponent, SpriteComponent

		template <typename Component>
		void ComponentDrawView(entt::registry& p_Registry, Entity p_Entity) {}

		template <typename... Component>
		static void ComponentView(entt::registry& p_Registry, Entity p_Entity)
		{
			(ComponentDrawView<Component>(p_Registry, p_Entity), ...);
		}

		static bool EntityHasComponent(entt::registry& p_Registry, Entity p_Entity)
		{
			for (auto&& [id, pool] : p_Registry.storage()) 
			{
				if (pool.contains(p_Entity))
					return true;
			}
			return false;
		}

		template<typename Component>
		void DisplayComponentEntry(const std::string& p_Name, Entity p_Entt) 
		{
			if (!p_Entt.HasComponent<Component>())
			{
				if (ImGui::MenuItem(p_Name.c_str()))
				{
					p_Entt.AddComponent<Component>();
					ImGui::CloseCurrentPopup();
				}
			}
		}

		template<typename Component, typename Function>
		static void DrawComponent(const std::string& p_Name, Entity p_Entity, Function p_Function, bool p_Settings = true)
		{
			if (p_Entity.HasComponent<Component>())
			{
				const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen
					| ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Framed |
					ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

				auto& component = p_Entity.GetComponent<Component>();
				ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

				ImGui::PushID(p_Name.c_str());

				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4, 4 });
				float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
				ImGui::Spacing();
				bool open = ImGui::TreeNodeEx((void*)typeid(Component).hash_code(),
					treeNodeFlags, p_Name.c_str());
				ImGui::PopStyleVar();

				bool removeComponent = false;
				if (p_Settings)
				{
					ImGui::SameLine(contentRegionAvailable.x - lineHeight);

					if (ImGui::Button("+", { lineHeight, lineHeight }))
						ImGui::OpenPopup("ComponentSettings");

					if (ImGui::BeginPopup("ComponentSettings"))
					{
						if (ImGui::MenuItem("Remove Component"))
							removeComponent = true;

						ImGui::EndPopup();
					}
				}

				if (open)
				{
					p_Function(component);
					ImGui::TreePop();
				}

				if (removeComponent)
					p_Entity.RemoveComponent<Component>();
				ImGui::PopID();
			}
		}

		template <>
		void ComponentDrawView<TransformComponent>(entt::registry& p_Registry, Entity p_Entity)
		{
			DrawComponent<TransformComponent>("Transform", p_Entity, 
			[](TransformComponent& p_Transform)
			{
				glm::vec3 translation = p_Transform.GetLocalTranslation();
				UI::DrawFloat3("Translation", translation);
				p_Transform.SetLocalTranslation(translation);

				glm::vec3 rotation = p_Transform.GetLocalRotation();
				UI::DrawFloat3("Rotation", rotation);
				p_Transform.SetLocalRotation(rotation);

				glm::vec3 scale = p_Transform.GetLocalScale();
				UI::DrawFloat3("Scale", scale, 1.0f);
				p_Transform.SetLocalScale(scale);
			});
		}

		template <>
		void ComponentDrawView<CameraComponent>(entt::registry& p_Registry, Entity p_Entity)
		{
			DrawComponent<CameraComponent>("Camera", p_Entity,
			[](CameraComponent& p_Component)
			{
				Camera& camera = p_Component.Camera;

				ImGui::Checkbox("Primary", &p_Component.Primary);

				const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
				const char* currentProjectionTypeString = projectionTypeStrings[camera.IsOrthographic()];
				if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
				{
					for (int i = 0; i < 2; i++)
					{
						bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
						if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
						{
							currentProjectionTypeString = projectionTypeStrings[i];
							camera.SetIsOrthographic(i == 1);
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
				}

				if (!camera.IsOrthographic())
				{
					float fov = camera.GetFOV();
					if (ImGui::DragFloat("FOV", &fov))
						camera.SetFOV(fov);
				}
				else
				{
					float orthoSize = camera.GetScale();
					if (ImGui::DragFloat("Size", &orthoSize))
						camera.SetScale(orthoSize);
				}

				float zoom = camera.GetZoom();
				if (ImGui::DragFloat("Zoom", &zoom, 0.01f, 0.1f))
					camera.SetZoom(zoom);

				float nearz = camera.GetNear();
				if (ImGui::DragFloat("Near", &nearz))
					camera.SetNear(nearz);

				float farz = camera.GetFar();
				if (ImGui::DragFloat("Far", &farz))
					camera.SetFar(farz);

				bool fixed = camera.IsAspectRatioFixed();
				ImGui::Checkbox("Fixed Aspect Ratio", &fixed);
				camera.SetFixAspectRatio(fixed);
			});
		}

		template <>
		void ComponentDrawView<SpriteComponent>(entt::registry& p_Registry, Entity p_Entity)
		{
			DrawComponent<SpriteComponent>("Sprite", p_Entity,
			[](SpriteComponent& p_Sprite)
			{
				UI::DrawColorEdit4("Color", p_Sprite.Color, 1.0f);

				// TODO: Textures
			});
		}

	} // namespace

	InspectorPanel::InspectorPanel()
		: EditorPanel("Inspector")
	{
	}

	void InspectorPanel::OnImgui()
	{
		if (!m_Context)
			return;

		Entity selectedEntt = m_Editor->GetSelected();


		ImGui::Begin(m_Name.c_str(), &m_Active);

		if (selectedEntt)
		{
			auto& registry = selectedEntt.GetScene()->GetRegistry();

			auto name = selectedEntt.GetName();

			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			{
				if (UI::InputText(name, "##InspectorNameChange"))
					selectedEntt.GetComponent<TagComponent>().Tag = name;
			}

			ImGui::BeginChild("Components", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_None);

			if (EntityHasComponent(registry, selectedEntt))
			{ 
				ComponentView<ALL_VIEW_COMPONENTS>(registry, selectedEntt);
			}

			ImGui::NewLine();
			ImGui::NewLine();

			float size = ImGui::GetContentRegionAvail().x;
			ImGui::PushItemWidth(size / 2.0f);
			ImGui::SetCursorPosX((size / 2.0f) - ImGui::GetFontSize() * 3.0f);
			{
				if (ImGui::Button("Add Component"))
					ImGui::OpenPopup("AddComponent");

				if (ImGui::BeginPopup("AddComponent"))
				{
					DisplayComponentEntry<TransformComponent>("Transform", selectedEntt);
					DisplayComponentEntry<CameraComponent>("Camera", selectedEntt);
					DisplayComponentEntry<SpriteComponent>("Sprite", selectedEntt);

					ImGui::EndPopup();
				}
			}

			ImGui::EndChild();
		}

		ImGui::End();
	}

} // namespace KTN
