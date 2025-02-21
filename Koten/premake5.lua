include "include-dependencies.lua"

project "Koten"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "ktnpch.h"
	pchsource "Source/ktnpch.cpp"

	files
	{
		"Source/**.h",
		"Source/**.cpp",

		"%{IncludeDir.optick}/**.h",
		"%{IncludeDir.optick}/**.cpp"
	}

	includedirs
	{
		"Source",
		"%{IncludeDir.glad}",
		"%{IncludeDir.glslang}",
		"%{IncludeDir.SPIRVCross}",
		"%{IncludeDir.glfw}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yamlcpp}",
		"%{IncludeDir.optick}"
	}

	links
	{
		"glad",
		"glslang",
		"SPIRVCross",
		"spdlog",
		"glfw",
		"stb",
		"imgui",
		"yaml-cpp"
	}

	defines { "KTN_EXPORT", "OPTICK_EXPORT", "KTN_PROFILE_ENABLED", "USE_OPTICK=1" }

	filter "action:vs*"
		buildoptions { "/Zc:preprocessor" }


	filter "files:Thirdparty/Optick/src/**.cpp"
		flags { "NoPCH" }

	filter "system:windows"
		systemversion "latest"
		buildoptions { "/utf-8", "/wd4251" }
		links { "opengl32.lib" }
		defines
		{
			"KTN_WINDOWS",
			"GLFW_INCLUDE_NONE",
			"UNICODE", 
			"_UNICODE" 
		}
	
	filter "system:linux"
        pic "on"
		links { "GL" }
		buildoptions { "-finput-charset=UTF-8", "-fexec-charset=UTF-8", "-Wno-effc++" }
		defines
		{
			"KTN_LINUX",
			"GLFW_INCLUDE_NONE"
		}

	filter "configurations:Debug"
		defines "KTN_DEBUG"
		runtime "Debug"
		symbols "on"
		optimize "off"

	filter "configurations:Release"
		defines "KTN_RELEASE"
		runtime "Release"
		symbols "on"
		optimize "speed"

	filter "configurations:Dist"
		defines "KTN_DIST"
		runtime "Release"
		symbols "off"
		optimize "full"
