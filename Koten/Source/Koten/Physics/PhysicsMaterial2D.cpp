#include "ktnpch.h"
#include "PhysicsMaterial2D.h"
#include "Koten/Project/Project.h"

// lib
#include <yaml-cpp/yaml.h>



namespace KTN
{
	void PhysicsMaterial2D::Serialize(const std::string& p_Path) const
	{
		KTN_PROFILE_FUNCTION();

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "PhysicsMaterial2D" << YAML::Value << Handle;
		out << YAML::Key << "Density" << YAML::Value << Density;
		out << YAML::Key << "Friction" << YAML::Value << Friction;
		out << YAML::Key << "Restitution" << YAML::Value << Restitution;
		out << YAML::Key << "RestitutionThreshold" << YAML::Value << RestitutionThreshold;
		out << YAML::EndMap;

		std::ofstream fout(p_Path);
		fout << out.c_str();

		KTN_CORE_INFO("PhysicsMaterial2D serialized to path: {}", p_Path);
	}

	void PhysicsMaterial2D::SerializeBin(std::ofstream& p_Out) const
	{
		KTN_PROFILE_FUNCTION();

		p_Out.write(reinterpret_cast<const char*>(&Density), sizeof(Density));
		p_Out.write(reinterpret_cast<const char*>(&Friction), sizeof(Friction));
		p_Out.write(reinterpret_cast<const char*>(&Restitution), sizeof(Restitution));
		p_Out.write(reinterpret_cast<const char*>(&RestitutionThreshold), sizeof(RestitutionThreshold));
	}

	void PhysicsMaterial2D::DeserializeBin(std::ifstream& p_In)
	{
		KTN_PROFILE_FUNCTION();

		p_In.read(reinterpret_cast<char*>(&Density), sizeof(Density));
		p_In.read(reinterpret_cast<char*>(&Friction), sizeof(Friction));
		p_In.read(reinterpret_cast<char*>(&Restitution), sizeof(Restitution));
		p_In.read(reinterpret_cast<char*>(&RestitutionThreshold), sizeof(RestitutionThreshold));
	}

	AssetHandle PhysicsMaterial2D::GetDefault()
	{
		KTN_PROFILE_FUNCTION();

		static auto path = (Project::GetAssetFileSystemPath("Materials") / "Default.ktasset").string();
		FileSystem::CreateDirectories(Project::GetAssetFileSystemPath("Materials").string());
		if (!AssetManager::Get()->HasAsset(AssetType::PhysicsMaterial2D, path))
		{
			if (FileSystem::Exists(path))
			{
				auto material = AssetManager::Get()->ImportAsset(AssetType::PhysicsMaterial2D, path);
				if (!material)
				{
					KTN_CORE_ERROR("Failed to import default PhysicsMaterial2D: {}", path);
					return (AssetHandle)0;
				}
				return material;
			}
			
			auto material = CreateRef<PhysicsMaterial2D>();
			material->Serialize(path);

			AssetHandle handle;
			AssetMetadata metadata = {};
			metadata.FilePath = path;
			metadata.Type = AssetType::PhysicsMaterial2D;
			auto success = AssetManager::Get()->ImportAsset(handle, metadata, material);
			if (!success)
			{
				KTN_CORE_ERROR("Failed to import default PhysicsMaterial2D: {}", path);
				return (AssetHandle)0;
			}

			return handle;
		}

		return AssetManager::Get()->GetHandleByPath(path);
	}

} // namespace KTN