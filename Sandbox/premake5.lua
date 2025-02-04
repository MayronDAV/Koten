project "Sandbox"
	kind "ConsoleApp"
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
		"%{IncludeDir.ktn}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yamlcpp}",
		"%{IncludeDir.tracy}"
	}

	links
	{
		"Koten",
		"imgui",
		"spdlog",
		"yaml-cpp",
		"tracy"
	}

	filter "system:windows"
		systemversion "latest"
		buildoptions { "/utf-8", "/wd4251" }
		defines
		{
			"KTN_WINDOWS",
			"UNICODE", 
			"_UNICODE" 
		}

		postbuildcommands 
		{
			("{COPYFILE} %{wks.location}/bin/" .. outputdir .. "/Koten/Koten.dll %{wks.location}/bin/" .. outputdir .. "/%{prj.name}/")
		}

	filter "system:linux"
		pic "on"
		systemversion "latest"
		buildoptions { "-finput-charset=UTF-8", "-fexec-charset=UTF-8", "-Wno-effc++" }
		defines "KTN_LINUX"

		postbuildcommands 
		{
			("{COPY} %{wks.location}/bin/" .. outputdir .. "/Koten/libKoten.so %{wks.location}/bin/" .. outputdir .. "/%{prj.name}/")
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