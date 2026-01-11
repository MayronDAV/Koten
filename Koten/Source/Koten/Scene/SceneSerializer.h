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

			void SerializeBin(std::ofstream& p_Out);
			bool DeserializeBin(std::ifstream& p_In);
			bool DeserializeBin(const Buffer& p_Buffer);
			static bool DeserializeBin(std::ifstream& p_In, Buffer& p_Buffer);

		private:
			Ref<Scene> m_Scene;
	};

} // namespace KTN
