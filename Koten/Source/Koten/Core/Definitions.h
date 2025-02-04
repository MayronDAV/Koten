#pragma once

// std
#include <string>
#include <cstdint>



namespace KTN
{
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


} // namespace KTN