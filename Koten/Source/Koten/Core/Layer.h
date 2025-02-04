#pragma once
#include "Base.h"
#include "Koten/Events/Event.h"

// std
#include <string>


namespace KTN
{
	class KTN_API Layer
	{
		public:
			Layer(const std::string& p_Name = "Layer") : m_DebugName(p_Name) {}
			virtual ~Layer() = default;

			virtual void OnAttach()					{}
			virtual void OnDetach()					{}
			virtual void OnUpdate()					{}
			virtual void OnRender()					{}
			virtual void OnImgui()					{}
			virtual void OnEvent(Event& p_Event)	{}

			inline const std::string& GetName() const { return m_DebugName; }

		protected:
			std::string m_DebugName;
	};

}