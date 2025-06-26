#pragma once
#include "Editor.h"


namespace KTN
{
	class ProjectExplorer : public Layer
	{
		public:
			ProjectExplorer();
			~ProjectExplorer();

			void OnImgui() override;

		private:
			std::string m_ProjectPath;
			bool m_ShouldOpenProject = false;
	};

} // namespace KTN