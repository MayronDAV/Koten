#include "InspectorPanel.h"
#include "Editor.h"
#include "AssetImporterPanel.h"

// lib
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>



namespace KTN
{
	namespace
	{
		#define ALL_VIEW_COMPONENTS TransformComponent, CameraComponent, SpriteComponent, LineRendererComponent, TextRendererComponent, Rigidbody2DComponent, BoxCollider2DComponent, CircleCollider2DComponent, ScriptComponent

		template <typename Component>
		void ComponentDrawView(InspectorPanel* p_This, entt::registry& p_Registry, Entity p_Entity) {}

		template <typename... Component>
		static void ComponentView(InspectorPanel* p_This, entt::registry& p_Registry, Entity p_Entity)
		{
			(ComponentDrawView<Component>(p_This, p_Registry, p_Entity), ...);
		}

		static bool EntityHasComponent(InspectorPanel* p_This, entt::registry& p_Registry, Entity p_Entity)
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
		void ComponentDrawView<TransformComponent>(InspectorPanel* p_This, entt::registry& p_Registry, Entity p_Entity)
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
		void ComponentDrawView<CameraComponent>(InspectorPanel* p_This, entt::registry& p_Registry, Entity p_Entity)
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
		void ComponentDrawView<SpriteComponent>(InspectorPanel* p_This, entt::registry& p_Registry, Entity p_Entity)
		{
			DrawComponent<SpriteComponent>("Sprite", p_Entity,
			[&](SpriteComponent& p_Sprite)
			{
				auto assetManager = Project::GetActive()->GetAssetManager();

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
						p_Sprite.Texture = 0;

					ImGui::SameLine();

					std::string path = p_Sprite.Texture != 0 ? AssetManager::Get()->GetMetadata(p_Sprite.Texture).FilePath : "";
					std::string text = path.empty() ? "Texture" : path.c_str();
					if (ImGui::Button(text.c_str()))
					{
						std::string path = "";
						if (FileDialog::Open("", "Assets", path) == FileDialogResult::SUCCESS)
						{
							// TODO: Create a placeholder for assets and use the AssetImporterPanel to import if needed!
							p_Sprite.Texture = p_Sprite.Texture = AssetManager::Get()->ImportAsset(AssetType::Texture2D, path);
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
								// TODO: Create a placeholder for assets and use the AssetImporterPanel to import if needed!
								p_Sprite.Texture = AssetManager::Get()->ImportAsset(AssetType::Texture2D, filepath.string());
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
		void ComponentDrawView<LineRendererComponent>(InspectorPanel* p_This, entt::registry& p_Registry, Entity p_Entity)
		{
			DrawComponent<LineRendererComponent>("LineRenderer", p_Entity,
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
		void ComponentDrawView<TextRendererComponent>(InspectorPanel* p_This, entt::registry& p_Registry, Entity p_Entity)
		{
			DrawComponent<TextRendererComponent>("TextRenderer", p_Entity,
			[&](TextRendererComponent& p_Text)
			{
				UI::InputText("String", p_Text.String, true, 0, 2.0f, true);
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
				if (ImGui::Button(ICON_MDI_RESTART))
				{
					p_Text.Font = DFFont::GetDefault();
				}
				ImGui::SameLine();
				const auto& filePath = AssetManager::Get()->GetMetadata(p_Text.Font).FilePath;
				if (ImGui::Button(filePath.c_str()))
				{
					std::string path = "";
					if (FileDialog::Open(".ttf", "Fonts", path) == FileDialogResult::SUCCESS)
					{
						p_Text.Font = AssetManager::Get()->ImportAsset(AssetType::Font, path);
					}
				}
				ImGui::PopStyleVar();

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						auto filepath = std::filesystem::path(path);
						if (filepath.extension() == ".ttf")
						{
							// TODO: Create a placeholder for assets and use the AssetImporterPanel to import if needed!
							p_Text.Font = AssetManager::Get()->ImportAsset(AssetType::Font, filepath.string());
						}
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::Spacing();
				UI::ColorEdit4("Color", p_Text.Color, 1.0f);
				UI::ColorEdit4("BgColor", p_Text.BgColor, 1.0f);
				UI::ColorEdit4("Char BgColor", p_Text.CharBgColor, 1.0f);

				ImGui::Spacing();

				ImGui::Checkbox("Draw Bg", &p_Text.DrawBg);
				ImGui::InputFloat("Kerning", &p_Text.Kerning);
				ImGui::InputFloat("Line Spacing", &p_Text.LineSpacing);

				ImGui::Spacing();
			});
		}

		template <>
		void ComponentDrawView<Rigidbody2DComponent>(InspectorPanel* p_This, entt::registry& p_Registry, Entity p_Entity)
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
		void ComponentDrawView<BoxCollider2DComponent>(InspectorPanel* p_This, entt::registry& p_Registry, Entity p_Entity)
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
		void ComponentDrawView<CircleCollider2DComponent>(InspectorPanel* p_This, entt::registry& p_Registry, Entity p_Entity)
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

		template <>
		void ComponentDrawView<ScriptComponent>(InspectorPanel* p_This, entt::registry& p_Registry, Entity p_Entity)
		{
			DrawComponent<ScriptComponent>("Script", p_Entity,
			[&](ScriptComponent& p_SC)
			{
				static const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
				bool scriptClassExists = ScriptEngine::EntityClassExists(p_SC.FullClassName);

				std::string id = "##ScriptClassSelector" + std::to_string(p_Entity.GetUUID());
				ImGui::PushID(id.c_str());

				ImGui::Text("Class");
				ImGui::SameLine();

				if (ImGui::Button(p_SC.FullClassName.c_str(), {0.0f, lineHeight}))
					ImGui::OpenPopup("ScriptClassSelector");

				if (ImGui::BeginPopup("ScriptClassSelector"))
				{
					KTN_PROFILE_FUNCTION();

					static char buffer[128];
					buffer[0] = '\0'; // Clear the buffer initially

					ImGui::Text(ICON_MDI_SEARCH_WEB);
					ImGui::SameLine();
					ImGui::InputText("##Search", buffer, sizeof(buffer), ImGuiInputTextFlags_AutoSelectAll);

					auto size = ImGui::GetContentRegionAvail();
					ImGui::SetNextWindowSizeConstraints({ size.x, 50.0f }, { size.x, 150.0f });
					ImGui::BeginChild("##ClassesList", {0, 0}, true);
					{
						auto& classesMap = ScriptEngine::GetEntityClasses();
						for (const auto& [name, classes] : classesMap)
						{
							if (buffer[0] != '\0')
							{
								std::string nameLower = name;
								std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

								char bufferLower[128];
								Strncpy(bufferLower, buffer, sizeof(bufferLower));
								std::transform(bufferLower, bufferLower + sizeof(bufferLower), bufferLower, ::tolower);

								if (nameLower.find(bufferLower) == std::string::npos)
									continue; // Skip if the name does not match the search
							}

							if (ImGui::Selectable(name.c_str(), p_SC.FullClassName == name))
							{
								p_SC.FullClassName = name;
								ImGui::CloseCurrentPopup();
							}
						}
					}
					ImGui::EndChild();

					ImGui::EndPopup();
				}


				// Fields
				Ref<ScriptInstance> scriptInstance = ScriptEngine::GetEntityScriptInstance(p_Entity.GetUUID());
				if (scriptInstance)
				{
					const auto& fields = scriptInstance->GetScriptClass()->GetFields();

					for (const auto& [name, field] : fields)
					{
						ImGui::Text(name.c_str());
						ImGui::SameLine();

						if (field.Type == ScriptFieldType::Float)
						{
							float data = scriptInstance->GetFieldValue<float>(name);

							if (field.IsPrivate)
							{
								if (field.ShowInEditor)
									ImGui::InputFloat(std::string("##Input" + name).c_str(), &data, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_ReadOnly);
								continue; // Skip the drag for private fields
							}

							if (ImGui::DragFloat(std::string("##Drag" + name).c_str(), &data))
							{
								scriptInstance->SetFieldValue(name, data);
							}
						}
					}
				}
				else if (scriptClassExists)
				{
					Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(p_SC.FullClassName);
					const auto& fields = entityClass->GetFields();

					auto& entityFields = ScriptEngine::GetScriptFieldMap(p_Entity);
					for (const auto& [name, field] : fields)
					{
						ImGui::Text(name.c_str());
						ImGui::SameLine();

						if (field.Type == ScriptFieldType::Float)
						{
							ScriptFieldInstance& fieldInstance = entityFields.find(name) != entityFields.end() ? entityFields.at(name) : entityFields[name];
							if (entityFields.find(name) == entityFields.end())
								fieldInstance.Field = field;

							float data = fieldInstance.GetValue<float>();

							if (field.IsPrivate)
							{
								if (field.ShowInEditor)
									ImGui::InputFloat(std::string("##Input" + name).c_str(), &data, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_ReadOnly);
								continue; // Skip the drag for private fields
							}

							if (ImGui::DragFloat(std::string("##Drag" + name).c_str(), &data))
								fieldInstance.SetValue(data);
						}
					}
				}

				ImGui::PopID();
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

		Entity selectedEntt = m_Editor->GetSelected();

		ImGui::Begin(m_Name.c_str(), &m_Active);

		if (selectedEntt)
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
				if (UI::InputText("##InspectorNameChange", name))
					selectedEntt.GetComponent<TagComponent>().Tag = name;

			#ifdef KTN_DEBUG
				auto id = (uint64_t)selectedEntt.GetUUID();
				UI::Tooltip(std::to_string(id).c_str());
			#endif
			}

			ImGui::BeginChild("Components", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_None);

			if (EntityHasComponent(this, registry, selectedEntt))
			{ 
				ComponentView<ALL_VIEW_COMPONENTS>(this, registry, selectedEntt);
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
					DisplayComponentEntry<LineRendererComponent>("LineRenderer", selectedEntt);
					DisplayComponentEntry<TextRendererComponent>("TextRenderer", selectedEntt);
					DisplayComponentEntry<Rigidbody2DComponent>("Rigidbody2D", selectedEntt);
					DisplayComponentEntry<BoxCollider2DComponent>("BoxCollider2D", selectedEntt);
					DisplayComponentEntry<CircleCollider2DComponent>("CircleCollider2D", selectedEntt);
					DisplayComponentEntry<ScriptComponent>("ScriptComponent", selectedEntt);

					ImGui::EndPopup();
				}
			}

			ImGui::EndChild();
		}

		ImGui::End();
	}

} // namespace KTN
