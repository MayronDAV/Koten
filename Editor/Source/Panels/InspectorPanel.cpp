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
		#define ALL_VIEW_COMPONENTS TransformComponent, CameraComponent, SpriteComponent, LineRendererComponent, Rigidbody2DComponent, BoxCollider2DComponent, CircleCollider2DComponent

		template <typename Component>
		void ComponentDrawView(entt::registry& p_Registry, Entity p_Entity) {}

		template <typename... Component>
		static void ComponentView(entt::registry& p_Registry, Entity p_Entity)
		{
			(ComponentDrawView<Component>(p_Registry, p_Entity), ...);
		}

		static bool EntityHasComponent(entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

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
			KTN_PROFILE_FUNCTION();

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
			KTN_PROFILE_FUNCTION();

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
			KTN_PROFILE_FUNCTION();

			DrawComponent<TransformComponent>("Transform", p_Entity, 
			[](TransformComponent& p_Transform)
			{
				glm::vec3 translation = p_Transform.GetLocalTranslation();
				if (UI::DragFloat3("Translation", translation))
					p_Transform.SetLocalTranslation(translation);

				glm::vec3 rotation = glm::degrees(p_Transform.GetLocalRotation());
				if (UI::DragFloat3("Rotation", rotation))
					p_Transform.SetLocalRotation(glm::radians(rotation));

				glm::vec3 scale = p_Transform.GetLocalScale();
				if (UI::DragFloat3("Scale", scale, 1.0f))
					p_Transform.SetLocalScale(scale);
			});
		}

		template <>
		void ComponentDrawView<CameraComponent>(entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			DrawComponent<CameraComponent>("Camera", p_Entity,
			[](CameraComponent& p_Component)
			{
				Camera& camera = p_Component.Camera;

				ImGui::Checkbox("Primary", &p_Component.Primary);

				int currentItem = (int)camera.IsOrthographic();
				static const char* items[] = { "Perspective", "Orthographic" };
				static const int itemsCount = IM_ARRAYSIZE(items);

				if (UI::Combo("Type", items[currentItem], items, itemsCount, &currentItem, -1))
				{
					camera.SetIsOrthographic(currentItem == 1);
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
			[&](SpriteComponent& p_Sprite)
			{
				
				int currentItem = (int)p_Sprite.Type;
				static const char* items[] = { "Quad", "Circle" };
				static const int itemsCount = IM_ARRAYSIZE(items);

				if (UI::Combo("Type", items[currentItem], items, itemsCount, &currentItem, -1))
				{
					p_Sprite.Type = (RenderType2D)currentItem;
				}

				UI::ColorEdit4("Color", p_Sprite.Color, 1.0f);

				ImGui::Spacing();

				if (p_Sprite.Type == RenderType2D::Circle)
				{
					ImGui::InputFloat("Thickness", &p_Sprite.Thickness);
					ImGui::InputFloat("Fade", &p_Sprite.Fade);
				}

				{
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

					if (ImGui::Button(ICON_MDI_CANCEL))
					{
						p_Sprite.Texture = nullptr;
						p_Sprite.Path = "";
					}

					ImGui::SameLine();

					std::string text = p_Sprite.Path.empty() ? "Texture" : p_Sprite.Path.c_str();
					if (ImGui::Button(text.c_str()))
					{
						std::string path = "";
						if (FileDialog::Open("", "Assets", path) == FileDialogResult::SUCCESS)
						{
							p_Sprite.Texture = TextureImporter::LoadTexture2D(path);
							p_Sprite.Path = path;
						}
					}

					ImGui::PopStyleVar();

					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
						{
							const wchar_t* path = (const wchar_t*)payload->Data;
							auto filepath = std::filesystem::path(path);
							if (filepath.extension() == ".png" || filepath.extension() == ".jpg" || filepath.extension() == ".jpeg")
							{
								p_Sprite.Texture = TextureImporter::LoadTexture2D(filepath.string());
								p_Sprite.Path = filepath.string();
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::Spacing();
				}
			
				const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen
					| ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Framed |
					ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4, 4 });
				bool open = ImGui::TreeNodeEx("UV", treeNodeFlags);
				ImGui::PopStyleVar();

				if (open)
				{
					ImGui::InputFloat2("Size", &p_Sprite.Size[0], "%.2f");
					UI::Tooltip("Sprite size, if 0.0 the texture width and height will be used");
					ImGui::Checkbox("By Size", &p_Sprite.BySize);
					UI::Tooltip("If true, the tile coord will be multiplied by the size");
					ImGui::InputFloat2("Offset", &p_Sprite.Offset[0], "%.2f");
					ImGui::InputFloat2("Scale", &p_Sprite.Scale[0], "%.2f");

					ImGui::TreePop();
				}
			});
		}

		template <>
		void ComponentDrawView<LineRendererComponent>(entt::registry& p_Registry, Entity p_Entity)
		{
			DrawComponent<LineRendererComponent>("Line Renderer", p_Entity,
			[](LineRendererComponent& p_Line)
			{
				ImGui::Checkbox("Primitive", &p_Line.Primitive);
				ImGui::InputFloat("Width", &p_Line.Width);
				UI::ColorEdit4("Color", p_Line.Color, 1.0f);

				glm::vec3 start = p_Line.Start;
				glm::vec3 end = p_Line.End;
				if (UI::DragFloat3("Start", start))
					p_Line.Start = start;
				if (UI::DragFloat3("End", end))
					p_Line.End = end;

			});
		}

		template <>
		void ComponentDrawView<Rigidbody2DComponent>(entt::registry& p_Registry, Entity p_Entity)
		{
			DrawComponent<Rigidbody2DComponent>("Rigidbody2D", p_Entity,
			[](Rigidbody2DComponent& p_RC)
			{
				int currentItem = (int)p_RC.Type;
				static const char* items[] = { "Static", "Dynamic", "Kinematic" };
				static const int itemsCount = IM_ARRAYSIZE(items);

				if (UI::Combo("Type", items[currentItem], items, itemsCount, &currentItem, 50.0f))
				{
					p_RC.Type = (Rigidbody2DComponent::BodyType)currentItem;
				}

				ImGui::Checkbox("Fixed Rotation", &p_RC.FixedRotation);
			});
		}

		template <>
		void ComponentDrawView<BoxCollider2DComponent>(entt::registry& p_Registry, Entity p_Entity)
		{
			DrawComponent<BoxCollider2DComponent>("BoxCollider2D", p_Entity,
			[](BoxCollider2DComponent& p_Box)
			{
				UI::InputFloat2("Offset", p_Box.Offset);
				UI::InputFloat2("Size", p_Box.Size, 0.5f);

				ImGui::InputFloat("Density", &p_Box.Density);
				ImGui::InputFloat("Friction", &p_Box.Friction);
				ImGui::InputFloat("Restitution", &p_Box.Restitution);
				ImGui::InputFloat("RestitutionThreshold", &p_Box.RestitutionThreshold);
			});
		}

		template <>
		void ComponentDrawView<CircleCollider2DComponent>(entt::registry& p_Registry, Entity p_Entity)
		{
			DrawComponent<CircleCollider2DComponent>("CircleCollider2D", p_Entity,
			[](CircleCollider2DComponent& p_Circle)
			{
				UI::InputFloat2("Offset", p_Circle.Offset);
				ImGui::InputFloat("Radius", &p_Circle.Radius);

				ImGui::InputFloat("Density", &p_Circle.Density);
				ImGui::InputFloat("Friction", &p_Circle.Friction);
				ImGui::InputFloat("Restitution", &p_Circle.Restitution);
				ImGui::InputFloat("RestitutionThreshold", &p_Circle.RestitutionThreshold);
			});
		}

	} // namespace

	InspectorPanel::InspectorPanel()
		: EditorPanel("Inspector")
	{
	}

	void InspectorPanel::OnImgui()
	{
		KTN_PROFILE_FUNCTION();

		if (!m_Context)
			return;

		Entity selectedEntt = m_Editor->GetSelected();


		ImGui::Begin(m_Name.c_str(), &m_Active);

		// TODO: Change this to lock the imgui items instead of just not showing them
		auto state = m_Editor->GetState();
		if (selectedEntt && state == RuntimeState::Edit)
		{
			auto& registry = selectedEntt.GetScene()->GetRegistry();
			if (!registry.valid(selectedEntt))
			{
				m_Editor->UnSelectEntt();
				ImGui::End();
				return;
			}

			auto name = selectedEntt.GetTag();

			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			{
				if (UI::InputText(name, "##InspectorNameChange"))
					selectedEntt.GetComponent<TagComponent>().Tag = name;

				// for debugging, maybe create a variable to toggle this.
				//auto id = (uint64_t)selectedEntt.GetUUID();
				//UI::Tooltip(std::to_string(id).c_str());
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
					DisplayComponentEntry<LineRendererComponent>("Line Renderer", selectedEntt);
					DisplayComponentEntry<Rigidbody2DComponent>("Rigidbody2D", selectedEntt);
					DisplayComponentEntry<BoxCollider2DComponent>("BoxCollider2D", selectedEntt);
					DisplayComponentEntry<CircleCollider2DComponent>("CircleCollider2D", selectedEntt);

					ImGui::EndPopup();
				}
			}

			ImGui::EndChild();
		}

		ImGui::End();
	}

} // namespace KTN
