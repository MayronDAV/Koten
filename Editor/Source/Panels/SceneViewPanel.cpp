#include "SceneViewPanel.h"
#include "Editor.h"

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

			UI::Image(m_MainTexture, { (float)m_Width, (float)m_Height });

			m_Width = (uint32_t)viewportSize.x;
			m_Height = (uint32_t)viewportSize.y;

			// Gizmos
			Entity selectedEntity = m_Editor->GetSelected();
			if (selectedEntity && m_GuizmoType != 0)
			{
				glm::vec2 viewportBounds[2] = {
					{ m_ViewportMinRegion.x + m_ViewportOffset.x, m_ViewportMinRegion.y + m_ViewportOffset.y },
					{ m_ViewportMaxRegion.x + m_ViewportOffset.x, m_ViewportMaxRegion.y + m_ViewportOffset.y }
				};

				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();

				ImGuizmo::SetRect(viewportBounds[0].x, viewportBounds[0].y, viewportBounds[1].x - viewportBounds[0].x, viewportBounds[1].y - viewportBounds[0].y);

				auto& tc = selectedEntity.GetComponent<TransformComponent>();
				glm::mat4 transform = tc.GetLocalMatrix();

				glm::mat4 cameraProjection = m_Context->GetProj();
				glm::mat4 cameraView = m_Context->GetView();

				ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
					(ImGuizmo::OPERATION)m_GuizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform));

				if (ImGuizmo::IsUsing())
				{
					glm::vec3 translation, rotation, scale;
					Math::Transform::Decompose(transform, translation, scale, rotation);

					glm::vec3 rt = tc.GetLocalRotation();
					glm::vec3 deltaRotation = rotation - rt;
					tc.SetLocalTranslation(translation);
					tc.SetLocalRotation(rt + deltaRotation);
					tc.SetLocalScale(scale);
				}
			}
		}
		ImGui::End();

		DrawGuizmoWidget();
	}

	void SceneViewPanel::OnUpdate()
	{
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
	}

	void SceneViewPanel::OnRender()
	{
		Renderer::Clear();

		m_Context->SetRenderTarget(m_MainTexture);
		m_Context->SetViewportSize(m_Width, m_Height);
		m_Context->OnUpdate();

		m_Context->OnRender();
	}

	void SceneViewPanel::DrawGuizmoWidget()
	{
		auto widgetSize = ImVec2(150, 40);

		ImVec2 widgetPos(
			m_ViewportOffset.x + 10,
			m_ViewportOffset.y + m_TitlebarHeight + 10
		);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 15.0f);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));

		ImGui::SetNextWindowPos(widgetPos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(widgetSize, ImGuiCond_Always);
		ImGui::Begin("Guizmo Widget", &m_Active, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar();
		{
			auto selectedColor = ImVec4(155.0f / 255.0f, 130.0f / 255.0f, 207.0f / 255.0f, 1.00f); // TODO: Change this when we have themes

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

			ImGui::PopStyleColor();
		}
		ImGui::End();
	}

} // namespace KTN
