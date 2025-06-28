#include "SceneViewPanel.h"
#include "Editor.h"
#include "Shortcuts.h"

// lib
#include <imgui_internal.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>




namespace KTN
{

	SceneViewPanel::SceneViewPanel()
		: EditorPanel("Scene View")
	{
	}

	void SceneViewPanel::OnImgui()
	{
		KTN_PROFILE_FUNCTION();

		auto& camera = m_Editor->GetCamera();
		glm::mat4 cameraProjection = camera->GetProjection();
		glm::mat4 cameraView = camera->GetView();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
		ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.0f, 0.0f, 0.0f, 1.0f });
		ImGui::Begin(m_Name.c_str(), &m_Active);
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
		{
			ImVec2 viewportSize = ImGui::GetContentRegionAvail();
			m_TitlebarHeight = ImGui::GetFrameHeight();
			m_ViewportMinRegion = ImGui::GetWindowContentRegionMin();
			m_ViewportMaxRegion = ImGui::GetWindowContentRegionMax();
			m_ViewportOffset = ImGui::GetWindowPos();

			m_HandleCameraEvents = ImGui::IsWindowFocused();

			UI::Image(m_MainTexture, { (float)m_Width, (float)m_Height });

			auto state = m_Editor->GetState();

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					if (state == RuntimeState::Play)
						Stop();

					const wchar_t* path = (const wchar_t*)payload->Data;
					auto filepath = std::filesystem::path(path);
					if (filepath.extension() == ".ktscn")
					{
						m_Editor->OpenScene(Project::GetActive()->GetAssetManager()->ImportAsset(AssetType::Scene, filepath.string()));
					}
				}
				ImGui::EndDragDropTarget();
			}

			m_Width = (uint32_t)viewportSize.x;
			m_Height = (uint32_t)viewportSize.y;

			if (state == RuntimeState::Edit)
			{
				glm::vec2 viewportBounds[2] = {
					{ m_ViewportMinRegion.x + m_ViewportOffset.x, m_ViewportMinRegion.y + m_ViewportOffset.y },
					{ m_ViewportMaxRegion.x + m_ViewportOffset.x, m_ViewportMaxRegion.y + m_ViewportOffset.y }
				};

				bool imguizmo = (m_GuizmoType != 0 && ImGuizmo::IsOver()) || ImGuizmo::IsUsing();
				if (Engine::GetSettings().MousePicking && !ImGui::IsDragDropActive() && !imguizmo)
				{
					auto [mx, my] = ImGui::GetMousePos();
					mx -= viewportBounds[0].x;
					my -= viewportBounds[0].y;
					if (Engine::GetAPI() == RenderAPI::OpenGL)
						my = m_Height - my;

					glm::ivec2 mouse = { (int)mx, (int)my };

					if ((mouse.x >= 0 && mouse.x < (int)m_Width) &&
						(mouse.y >= 0 && mouse.y < (int)m_Height))
					{
						if (Input::IsMouseButtonPressed(Mouse::Button_Left))
						{
							auto texture = Renderer::GetPickingTexture();
							int id = static_cast<int>((intptr_t)RendererCommand::ReadPixel(texture, mouse.x, mouse.y));
							if (id >= 0)
								m_Editor->SetSelectedEntt({ (entt::entity)id, m_Context.get() });
							else
								m_Editor->UnSelectEntt();
						}
					}
				}


				// Gizmos
				Entity selectedEntity = m_Editor->GetSelected();
				if (selectedEntity && m_GuizmoType != 0)
				{

					ImGuizmo::SetOrthographic(camera->GetMode() == EditorCameraMode::TWODIM);
					ImGuizmo::SetDrawlist();

					ImGuizmo::SetRect(viewportBounds[0].x, viewportBounds[0].y, viewportBounds[1].x - viewportBounds[0].x, viewportBounds[1].y - viewportBounds[0].y);

					auto tc = selectedEntity.TryGetComponent<TransformComponent>();
					if (tc)
					{
						glm::mat4 transform = tc->GetLocalMatrix();

						ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
							(ImGuizmo::OPERATION)m_GuizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform));

						if (ImGuizmo::IsUsing())
						{
							glm::vec3 translation, rotation, scale;
							Math::Transform::Decompose(transform, translation, scale, rotation);

							tc->SetLocalTranslation(translation);
							tc->SetLocalRotation(rotation);
							tc->SetLocalScale(scale);
						}
					}
				}
			}
		}
		ImGui::End();

		ToolWidget();

		UIWidget();
	}

	void SceneViewPanel::OnUpdate()
	{
		KTN_PROFILE_FUNCTION();

		TextureSpecification tspec = {};
		tspec.Width = m_Width;
		tspec.Height = m_Height;
		tspec.Format = TextureFormat::RGBA32_FLOAT;
		tspec.Usage = TextureUsage::TEXTURE_COLOR_ATTACHMENT;
		tspec.Samples = 1;
		tspec.GenerateMips = false;
		tspec.AnisotropyEnable = false;
		tspec.DebugName = "MainTexture";

		m_MainTexture = Texture2D::Get(tspec);

		auto state = m_Editor->GetState();

		m_Context->SetRenderTarget(m_MainTexture);
		if (state == RuntimeState::Edit)
		{
			auto& camera = m_Editor->GetCamera();
			camera->SetViewportSize(m_Width, m_Height);
			Application::Get().GetImGui()->BlockEvents(m_HandleCameraEvents);
			camera->SetHandleEvents(m_HandleCameraEvents);
			m_Context->SetViewportSize(m_Width, m_Height);
			m_Context->OnUpdate();
		}
		else if (state == RuntimeState::Play)
		{
			m_ActiveScene->SetViewportSize(m_Width, m_Height);
			m_ActiveScene->OnUpdateRuntime();
		}
		else if (state == RuntimeState::Simulate)
		{
			auto& camera = m_Editor->GetCamera();
			camera->SetViewportSize(m_Width, m_Height);
			Application::Get().GetImGui()->BlockEvents(m_HandleCameraEvents);
			camera->SetHandleEvents(m_HandleCameraEvents);
			m_ActiveScene->SetViewportSize(m_Width, m_Height);
			m_ActiveScene->OnUpdateSimulation();
		}


		if (state == RuntimeState::Edit && !Input::IsMouseButtonPressed(Mouse::Button_Right))
		{
			if (Shortcuts::IsActionPressed("Guizmo None"))
				m_GuizmoType = 0;

			if (Shortcuts::IsActionPressed("Guizmo Translate"))
				m_GuizmoType = ImGuizmo::TRANSLATE;

			if (Shortcuts::IsActionPressed("Guizmo Rotate"))
				m_GuizmoType = ImGuizmo::ROTATE;

			if (Shortcuts::IsActionPressed("Guizmo Scale"))
				m_GuizmoType = ImGuizmo::SCALE;

			if (Shortcuts::IsActionPressed("Guizmo Universal"))
				m_GuizmoType = ImGuizmo::UNIVERSAL;

			if (Shortcuts::IsActionPressed("Play"))
			{
				Play();
			}
		}

		if (state != RuntimeState::Edit)
		{
			if (Shortcuts::IsActionPressed("Stop"))
			{
				Stop();
			}
		}
	}

	void SceneViewPanel::OnRender()
	{
		KTN_PROFILE_FUNCTION();

		Renderer::Clear();

		auto state = m_Editor->GetState();
		if (state == RuntimeState::Edit)
		{
			auto& camera = m_Editor->GetCamera();
			m_Context->OnRender(camera->GetProjection(), camera->GetView());
		}
		else if (state == RuntimeState::Simulate)
		{
			auto& camera = m_Editor->GetCamera();
			m_ActiveScene->OnRender(camera->GetProjection(), camera->GetView());
		}
		else if (state == RuntimeState::Play)
		{
			m_ActiveScene->OnRenderRuntime();
		}
	}

	void SceneViewPanel::ToolWidget()
	{
		KTN_PROFILE_FUNCTION();

		auto widgetSize = ImVec2(190, 40);

		ImVec2 widgetPos(
			m_ViewportOffset.x + 10,
			m_ViewportOffset.y + m_TitlebarHeight + 10
		);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));

		ImGui::SetNextWindowPos(widgetPos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(widgetSize, ImGuiCond_Always);
		ImGui::Begin("Guizmo Widget", &m_Active, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		{
			//auto selectedColor = ImVec4(155.0f / 255.0f, 130.0f / 255.0f, 207.0f / 255.0f, 1.00f);
			ImGuiContext& g = *GImGui;
			const ImGuiStyle& style = g.Style;
			auto selectedColor = style.Colors[ImGuiCol_FrameBgActive];

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			bool selected = false;

			{
				selected = m_GuizmoType == 0;
				if (selected)
					ImGui::PushStyleColor(ImGuiCol_Text, selectedColor);
				ImGui::SameLine();
				if (ImGui::Button(ICON_MDI_CURSOR_DEFAULT))
					m_GuizmoType = 0;

				if (selected)
					ImGui::PopStyleColor();

				UI::Tooltip("Select");
			}
			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();

			{
				selected = m_GuizmoType == ImGuizmo::TRANSLATE;
				if (selected)
					ImGui::PushStyleColor(ImGuiCol_Text, selectedColor);
				ImGui::SameLine();
				if (ImGui::Button(ICON_MDI_ARROW_ALL))
					m_GuizmoType = ImGuizmo::TRANSLATE;

				if (selected)
					ImGui::PopStyleColor();

				UI::Tooltip("Translate");
			}

			{
				selected = m_GuizmoType == ImGuizmo::ROTATE;
				if (selected)
					ImGui::PushStyleColor(ImGuiCol_Text, selectedColor);

				ImGui::SameLine();
				if (ImGui::Button(ICON_MDI_ROTATE_ORBIT))
					m_GuizmoType = ImGuizmo::ROTATE;

				if (selected)
					ImGui::PopStyleColor();

				UI::Tooltip("Rotate");
			}

			{
				selected = m_GuizmoType == ImGuizmo::SCALE;
				if (selected)
					ImGui::PushStyleColor(ImGuiCol_Text, selectedColor);

				ImGui::SameLine();
				if (ImGui::Button(ICON_MDI_ARROW_EXPAND_ALL))
					m_GuizmoType = ImGuizmo::SCALE;

				if (selected)
					ImGui::PopStyleColor();

				UI::Tooltip("Scale");
			}

			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();

			{
				selected = m_GuizmoType == ImGuizmo::UNIVERSAL;
				if (selected)
					ImGui::PushStyleColor(ImGuiCol_Text, selectedColor);

				ImGui::SameLine();
				if (ImGui::Button(ICON_MDI_CROP_ROTATE))
					m_GuizmoType = ImGuizmo::UNIVERSAL;

				if (selected)
					ImGui::PopStyleColor();

				UI::Tooltip("Universal");
			}

			ImGui::PopStyleColor();
		}
		ImGui::End();

		ImVec2 fpsPos(
			m_ViewportOffset.x + 10,
			m_ViewportOffset.y + m_TitlebarHeight + 10 + widgetSize.y + 10
		);

		ImGui::SetNextWindowPos(fpsPos, ImGuiCond_Always);
		ImGui::SetNextWindowSize({ 100, 20 }, ImGuiCond_Always);
		ImGui::Begin("Fps", &m_Active, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		{
			ImGui::Text("FPS: %u", Engine::GetStats().FramesPerSecond);
		}
		ImGui::End();
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar();
	}
	
	void SceneViewPanel::UIWidget()
	{
		KTN_PROFILE_FUNCTION();

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;

		bool isPaused = m_ActiveScene && m_ActiveScene->IsPaused();

		auto widgetSize = ImVec2((isPaused ? 100.0f : 70.0f ) + (style.WindowPadding.x / 2.0f), 35.0f + (style.WindowPadding.y / 2.0f));

		ImVec2 widgetPos(
			m_ViewportOffset.x + ((float)m_Width / 2.0f) - ((float)widgetSize.x / 2.0f),
			m_ViewportOffset.y + m_TitlebarHeight + 10
		);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));

		ImGui::SetNextWindowPos(widgetPos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(widgetSize, ImGuiCond_Always);
		ImGui::Begin("UIWidget", &m_Active, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar();
		{
			auto state = m_Editor->GetState();

			bool hasPlayButton = state == RuntimeState::Edit || state == RuntimeState::Play;
			bool hasSimulateButton = state == RuntimeState::Edit || state == RuntimeState::Simulate;
			bool hasPauseButton = state != RuntimeState::Edit;

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.5f));

			if (hasPlayButton)
			{
				std::string icon = state != RuntimeState::Play ? ICON_MDI_PLAY : ICON_MDI_STOP;

				if (ImGui::Button(icon.c_str()))
				{
					if (state == RuntimeState::Play)
						Stop();
					else if (state == RuntimeState::Edit)
						Play();
				}

				UI::Tooltip(state != RuntimeState::Play  ? "Play" : "Stop");
			}

			if (hasSimulateButton)
			{
				if (hasPlayButton)
					ImGui::SameLine();

				// TODO: find an icon for simulate button
				std::string icon = state != RuntimeState::Simulate ? ICON_MDI_PLAY_BOX_OUTLINE : ICON_MDI_STOP;

				if (ImGui::Button(icon.c_str()))
				{
					if (state == RuntimeState::Simulate)
						Stop();
					else if (state == RuntimeState::Edit)
						Simulate();
				}

				UI::Tooltip(state != RuntimeState::Simulate ? "Simulate" : "Stop");
			}

			if (hasPauseButton)
			{
				ImGui::SameLine();

				std::string icon = ICON_MDI_PAUSE;
				if (ImGui::Button(icon.c_str()))
				{
					TogglePause();
				}

				UI::Tooltip("Pause");
			}

			if (isPaused)
			{
				ImGui::SameLine();

				std::string icon = ICON_MDI_STEP_FORWARD;
				if (ImGui::Button(icon.c_str()))
				{
					m_ActiveScene->Step();
				}

				UI::Tooltip("Step");
			}

			ImGui::PopStyleColor();
		}
		ImGui::End();
	}

	void SceneViewPanel::TogglePause()
	{
		KTN_PROFILE_FUNCTION();

		bool paused = !m_ActiveScene->IsPaused();
		m_ActiveScene->SetIsPaused(paused);
	}

	void SceneViewPanel::Play()
	{
		KTN_PROFILE_FUNCTION();

		if (m_Editor->GetState() == RuntimeState::Play) return;

		m_ActiveScene = Scene::Copy(m_Context);
		m_ActiveScene->OnRuntimeStart();

		m_Editor->SetState(RuntimeState::Play);
	}

	void SceneViewPanel::Simulate()
	{
		KTN_PROFILE_FUNCTION();

		if (m_Editor->GetState() == RuntimeState::Simulate) return;

		m_ActiveScene = Scene::Copy(m_Context);
		m_ActiveScene->OnSimulationStart();

		m_Editor->SetState(RuntimeState::Simulate);
	}

	void SceneViewPanel::Stop()
	{
		KTN_PROFILE_FUNCTION();

		auto state = m_Editor->GetState();
		if (state == RuntimeState::Edit) return;

		if (state == RuntimeState::Play)
			m_ActiveScene->OnRuntimeStop();
		if (state == RuntimeState::Simulate)
			m_ActiveScene->OnSimulationStop();
		m_ActiveScene = nullptr;

		m_Editor->SetState(RuntimeState::Edit);
	}

} // namespace KTN
