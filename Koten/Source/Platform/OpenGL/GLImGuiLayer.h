#pragma once

#include "Koten/ImGui/ImGuiLayer.h"



namespace KTN
{
	class GLImGuiLayer : public ImGuiLayer
	{
		public:
			GLImGuiLayer() = default;
			~GLImGuiLayer() override = default;

			void OnAttach() override;
			void OnDetach() override;

			void Begin() override;
			void End() override;
	};

} // namespace KTN
