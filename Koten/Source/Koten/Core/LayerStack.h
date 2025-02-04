#pragma once
#include "Base.h"
#include "Layer.h"

// std
#include <vector>



namespace KTN
{
	class KTN_API LayerStack
	{
		public:
			LayerStack() = default;
			~LayerStack();

			void PushLayer(Layer* p_Layer);
			void PushOverlay(Layer* p_Overlay);
			void PopLayer(Layer* p_Layer);
			void PopOverlay(Layer* p_Overlay);

			std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
			std::vector<Layer*>::iterator end() { return m_Layers.end(); }
			std::vector<Layer*>::reverse_iterator rbegin() { return m_Layers.rbegin(); }
			std::vector<Layer*>::reverse_iterator rend() { return m_Layers.rend(); }

			std::vector<Layer*>::const_iterator begin() const { return m_Layers.begin(); }
			std::vector<Layer*>::const_iterator end()	const { return m_Layers.end(); }
			std::vector<Layer*>::const_reverse_iterator rbegin() const { return m_Layers.rbegin(); }
			std::vector<Layer*>::const_reverse_iterator rend() const { return m_Layers.rend(); }

		private:
			std::vector<Layer*> m_Layers;
			uint32_t m_LayerInsertIndex = 0;
	};

}