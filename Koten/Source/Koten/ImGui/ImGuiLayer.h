#pragma once
#include "Koten/Core/Layer.h"

// lib
#include <imgui.h>



namespace KTN
{
	class KTN_API ImGuiLayer : public Layer
	{
		public:
			ImGuiLayer() : Layer("ImGuiLayer") {}
			virtual ~ImGuiLayer() = default;

			void BlockEvents(bool p_Block) { m_BlockEvents = p_Block; }

			virtual void Begin() = 0;
			virtual void End() = 0;

			void OnEvent(Event& p_Event) override;
			void OnUpdate() override;

			ImGuiContext* GetCurrentContext();

			uint32_t GetActiveWidgetID() const;

			static Ref<ImGuiLayer> Create();

		protected:
			void Init();
			void NewFrame();
			void EndFrame();
			void Shutdown();

		private:
			void AddFonts();

		private:
			float m_FontSize = 14.0f;
			bool  m_BlockEvents = true;
	};

} // namespace KTN