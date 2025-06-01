#pragma once

#include "Project.h"

namespace KTN
{
	class KTN_API ProjectSerializer
	{
		public:
			ProjectSerializer(const Ref<Project>& p_Project) : m_Project(p_Project) {}

			bool Serialize(const std::filesystem::path& p_Path);
			bool Deserialize(const std::filesystem::path& p_Path);

		private:
			Ref<Project> m_Project;
	};

} // namespace KTN
