include "include-dependencies.lua"

project "Koten"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"Source/**.h",
		"Source/**.cpp",
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
		"%{IncludeDir.tracy}"
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
		"yaml-cpp",
		"tracy"
	}

	defines "KTN_EXPORT"

	filter "system:windows"
		systemversion "latest"
		links { "opengl32.lib" }
		defines
		{
			"KTN_WINDOWS",
			"GLFW_INCLUDE_NONE"
		}
	
	filter "system:linux"
        pic "on"
		links { "GL", "X11", "Xrandr", "Xi", "dl", "pthread" }
		defines
		{
			"KTN_LINUX",
			"GLFW_INCLUDE_NONE"
		}

	filter "configurations:Debug"
		defines { "KTN_DEBUG", "_DEBUG", "KTN_PROFILE_ENABLED", "TRACY_ENABLE", "TRACY_ON_DEMAND" }
		runtime "Debug"
		symbols "on"
		optimize "off"

	filter "configurations:Release"
		defines { "KTN_RELEASE", "NDEBUG", "KTN_PROFILE_ENABLED", "TRACY_ENABLE", "TRACY_ON_DEMAND" }
		runtime "Release"
		symbols "on"
		optimize "speed"

	filter "configurations:Dist"
		defines { "KTN_DIST", "NDEBUG" }
		runtime "Release"
		symbols "off"
		optimize "full"
