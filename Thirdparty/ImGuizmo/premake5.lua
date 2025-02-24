project "ImGuizmo"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/Thirdparty/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/Thirdparty/%{prj.name}")

	
	files
	{
		"*.h",
		"*.cpp",
	}

	includedirs
	{
		".",
		"%{IncludeDir.imgui}"
	}

	links
	{
		"imgui"
	}

	filter "system:windows"
		systemversion "latest"

	filter "system:linux"
		pic "on"
		systemversion "latest"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		optimize "off"

	filter "configurations:Release"
		runtime "Release"
		symbols "on"
		optimize "speed"

	filter "configurations:Dist"
		runtime "Release"
		symbols "off"
		optimize "full"
