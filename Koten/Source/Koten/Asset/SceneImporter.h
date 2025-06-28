#pragma once
#include "Asset.h"
#include "Koten/Scene/Scene.h"



namespace KTN
{
	class KTN_API SceneImporter
	{
		public:
			static Ref<Scene> ImportScene(AssetHandle p_Handle, const AssetMetadata& p_Metadata);

			static Ref<Scene> LoadScene(const std::string& p_Path);

			static void SaveScene(Ref<Scene> p_Scene, const std::string& p_Path);
	};
} // namespace KTN