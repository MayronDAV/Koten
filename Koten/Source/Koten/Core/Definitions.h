#pragma once
#include "Koten/Core/Log.h"

// std
#include <string>
#include <cstdint>



namespace KTN
{
	#pragma region Enums

	enum class RenderAPI : uint8_t
	{
		None = 0, OpenGL, Vulkan
	};

	enum class WindowMode : uint8_t
	{
		Windowed = 0,
		Fullscreen,
		Borderless
	};

	enum class CursorMode : uint8_t
	{
		Normal = 0, Hidden, Disabled
	};

	enum class ShaderType : uint8_t
	{
		Vertex = 0,
		Fragment,
		Compute,
		Geometry,
		TessControl,
		TessEvaluation
	};

	enum class DescriptorType : uint8_t
	{
		UNIFORM_BUFFER,
		STORAGE_BUFFER,
		STORAGE_IMAGE,
		IMAGE_SAMPLER
	};

	#pragma endregion

	#pragma region Structs

	struct WindowResolution
	{
		int Width;
		int Height;
		int RefreshRate;
	};

	struct WindowSpecification
	{
		std::string Title		= "Koten";
		uint32_t Width			= 800;
		uint32_t Height			= 600;
		WindowMode Mode			= WindowMode::Windowed;
		bool Resizable			= true;
		bool Maximize			= false;
		bool Center				= true;
		bool Vsync				= false;
		std::string IconPath	= "";
	};

	struct MemberInfo
	{
		size_t Size;
		size_t Offset;
		std::string Name;
		std::string FullName;
	};

	struct DescriptorInfo
	{
		std::string Name;
		uint32_t Binding = 0u;
		size_t Offset = 0ul;
		size_t Size;
		DescriptorType Type;
		ShaderType Stage;

		std::vector<MemberInfo> Members;
	};

	struct PushConstant
	{
		std::string Name;
		uint32_t Binding = 0u;
		uint8_t* Data;
		size_t Size;
		size_t Offset = 0ul;
		ShaderType Stage;

		std::vector<MemberInfo> Members;

		inline void SetValue(const std::string& p_Name, void* p_Value)
		{
			for (auto& member : Members)
			{
				if (member.Name == p_Name)
				{
					memcpy(Data + member.Offset, p_Value, member.Size);
					return;
				}
			}

			KTN_CORE_ERROR("Push Constant not found: {}", p_Name);
		}

		~PushConstant()
		{
			if (Data)
			{
				delete Data;
				Data = nullptr;
			}
		}
	};

	#pragma endregion

} // namespace KTN