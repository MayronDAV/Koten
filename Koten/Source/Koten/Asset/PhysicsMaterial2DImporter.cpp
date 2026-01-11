#include "ktnpch.h"
#include "PhysicsMaterial2DImporter.h"

// lib
#include <yaml-cpp/yaml.h>



namespace KTN
{

    Ref<PhysicsMaterial2D> PhysicsMaterial2DImporter::ImportMaterial(AssetHandle p_Handle, const AssetMetadata& p_Metadata)
    {
		KTN_PROFILE_FUNCTION();

		if (p_Metadata.Type != AssetType::PhysicsMaterial2D)
		{
			KTN_CORE_ERROR("Invalid asset type for PhysicsMaterial2D import: {}", GetAssetTypeName(p_Metadata.Type));
			return nullptr;
		}

		Ref<PhysicsMaterial2D> material = nullptr;
		if (!p_Metadata.FilePath.empty())
			material = LoadMaterial(p_Metadata.FilePath);
		else
			material = CreateRef<PhysicsMaterial2D>();

		if (material) material->Handle = p_Handle;
		return material;
    }

	Ref<PhysicsMaterial2D> PhysicsMaterial2DImporter::ImportMaterialFromMemory(AssetHandle p_Handle, const AssetMetadata& p_Metadata, const Buffer& p_Data)
	{
		KTN_PROFILE_FUNCTION();

		if (p_Metadata.Type != AssetType::PhysicsMaterial2D)
		{
			KTN_CORE_ERROR("Invalid asset type for PhysicsMaterial2D import: {}", GetAssetTypeName(p_Metadata.Type));
			return nullptr;
		}

		BufferReader reader(p_Data);

		Ref<PhysicsMaterial2D> material = CreateRef<PhysicsMaterial2D>();
		material->Handle = p_Handle;

		material->Friction = reader.Read<float>();
		material->Restitution = reader.Read<float>();
		material->RestitutionThreshold = reader.Read<float>();

		return material;
	}

	Ref<PhysicsMaterial2D> PhysicsMaterial2DImporter::LoadMaterial(const std::string& p_Path)
	{
		KTN_PROFILE_FUNCTION();

		if (FileSystem::GetExtension(p_Path) != ".ktasset")
		{
			KTN_CORE_ERROR("Invalid file extension, it should be .ktasset!");
			return nullptr;
		}

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(p_Path);
		}
		catch (YAML::ParserException e)
		{
			KTN_CORE_ERROR("Failed to load .ktasset file '{0}'\n     {1}", p_Path, e.what());
			return nullptr;
		}

		auto material = CreateRef<PhysicsMaterial2D>();

		if (!data["PhysicsMaterial2D"])
			return nullptr;

		auto handle = data["PhysicsMaterial2D"].as<AssetHandle>();
		if (handle != 0)
			material->Handle = handle;

		if (data["Friction"]) material->Friction = data["Friction"].as<float>();
		if (data["Restitution"]) material->Restitution = data["Restitution"].as<float>();
		if (data["RestitutionThreshold"]) material->RestitutionThreshold = data["RestitutionThreshold"].as<float>();

		return material;
	}

} // namespace KTN
