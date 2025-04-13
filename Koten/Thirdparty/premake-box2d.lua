project "box2d"
	kind "StaticLib"
	language "C"
	cdialect "C17"
	staticruntime "off"
	warnings "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/Thirdparty/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/Thirdparty/%{prj.name}")

	files
	{
		"box2d/include/**.h",
		"box2d/src/**.h",
		"box2d/src/**.natvis",
		"box2d/src/**.c"
	}

	includedirs
	{
		"box2d",
		"box2d/include",
		"box2d/src"
	}

	filter "system:linux"
		pic "on"
		systemversion "latest"
		linkoptions { "-Wmissing-prototypes" }


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
