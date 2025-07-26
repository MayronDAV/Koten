#include "ktnpch.h"
#include "LayerStack.h"




namespace KTN
{
	LayerStack::~LayerStack()
	{
		KTN_PROFILE_FUNCTION();

		for (const Ref<Layer>& layer : m_Layers)
			layer->OnDetach();

		m_Layers.clear();
	}

	void LayerStack::Clear()
	{
		KTN_PROFILE_FUNCTION();

		for (const Ref<Layer>& layer : m_Layers)
			layer->OnDetach();

		m_Layers.clear();
		m_LayerInsertIndex = 0;
	}

	void LayerStack::PushLayer(const Ref<Layer>& p_Layer)
	{
		KTN_PROFILE_FUNCTION();

		m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, p_Layer);
		m_LayerInsertIndex++;
	}

	void LayerStack::PushOverlay(const Ref<Layer>& p_Overlay)
	{
		KTN_PROFILE_FUNCTION();

		m_Layers.emplace_back(p_Overlay);
	}

	void LayerStack::PopLayer(const Ref<Layer>& p_Layer)
	{
		KTN_PROFILE_FUNCTION();

		auto it = std::find(m_Layers.begin(), m_Layers.end(), p_Layer);
		if (it != m_Layers.end())
		{
			m_Layers.erase(it);
			m_LayerInsertIndex--;
		}
	}

	void LayerStack::PopOverlay(const Ref<Layer>& p_Overlay)
	{
		KTN_PROFILE_FUNCTION();

		auto it = std::find(m_Layers.begin(), m_Layers.end(), p_Overlay);
		if (it != m_Layers.end())
			m_Layers.erase(it);
	}

	void LayerStack::PopLayer(uint32_t p_Index)
	{
		KTN_PROFILE_FUNCTION();

		if (p_Index < m_Layers.size())
		{
			m_Layers.erase(m_Layers.begin() + p_Index);
			if (p_Index < m_LayerInsertIndex)
				m_LayerInsertIndex--;

			return;
		}

		KTN_CORE_ERROR("Index out of bounds in LayerStack::PopLayer");
	}

	void LayerStack::PopOverlay(uint32_t p_Index)
	{
		KTN_PROFILE_FUNCTION();

		if (p_Index < m_Layers.size())
		{
			m_Layers.erase(m_Layers.begin() + p_Index);
			return;
		}
		
		KTN_CORE_ERROR("Index out of bounds in LayerStack::PopOverlay");
	}

} // namespace KTN
