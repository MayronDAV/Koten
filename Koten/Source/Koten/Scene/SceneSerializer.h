#pragma once

#include "Scene.h"
#include "Entity.h"


namespace KTN
{
	class KTN_API SceneSerializer
	{
		public:
			SceneSerializer(const Ref<Scene>& p_Scene);

			void Serialize(const std::string& p_Filepath);
			bool Deserialize(const std::string& p_Filepath);

		private:
			Ref<Scene> m_Scene;
	};

} // namespace KTN
