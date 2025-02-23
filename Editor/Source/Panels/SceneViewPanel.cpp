#include "SceneViewPanel.h"
#include "Editor.h"

// lib
#include <imgui.h>



namespace KTN
{
	SceneViewPanel::SceneViewPanel()
		: EditorPanel("Scene View")
	{
	}

	void SceneViewPanel::OnImgui()
	{
		ImGui::SetNextWindowSizeConstraints({ 400.0f, 400.0f }, { FLT_MAX, FLT_MAX });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
		ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.0f, 0.0f, 0.0f, 1.0f });
		ImGui::Begin(m_Name.c_str());
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
		{
			ImVec2 viewportSize = ImGui::GetContentRegionAvail();

			UI::Image(m_MainTexture, { (float)m_Width, (float)m_Height });

			m_Width = (uint32_t)std::max(viewportSize.x, 400.0f);
			m_Height = (uint32_t)std::max(viewportSize.y, 400.0f);
		}
		ImGui::End();
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
		// TEMP
		m_Context->OnUpdate();

		m_Context->OnRender();
	}

} // namespace KTN
