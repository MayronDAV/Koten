#pragma once
#include "Koten/Scene/Scene.h"

// std
#include <string>



namespace KTN
{
	class Editor;

	class EditorPanel
	{
		public:
			EditorPanel(const std::string& p_Name) : m_Name(p_Name) {}
			virtual ~EditorPanel() = default;

			void SetEditor(Editor* p_Editor) { m_Editor = p_Editor; }
			void SetActive(bool p_Active) { m_Active = p_Active; }
			void SetContext(const Ref<Scene>& p_Context) { m_Context = p_Context; }

			virtual void OnImgui() {}
			virtual void OnUpdate() {}
			virtual void OnRender() {}

			const std::string& GetName() const { return m_Name; }
			Editor* GetEditor() { return m_Editor; }
			bool IsActive() const { return m_Active; }

		protected:
			bool m_Active = true;
			std::string m_Name;
			Ref<Scene> m_Context = nullptr;

			Editor* m_Editor = nullptr;
	};

} // namespace KTN
