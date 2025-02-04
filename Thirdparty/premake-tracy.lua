
project "tracy"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/Thirdparty/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/Thirdparty/%{prj.name}")

	files
	{
		"tracy/public/**.hpp",
		"tracy/public/**.h",
		"tracy/public/TracyClient.cpp"
	}

	filter "system:windows"
		systemversion "latest"
		defines { "NOMINMAX" }
		links { "ws2_32", "dbghelp" }

	filter "system:linux"
		pic "on"
		systemversion "latest"

	filter "system:bsd"
		pic "on"
		systemversion "latest"
		links { "execinfo" }

	filter "configurations:Debug"
		defines { "_DEBUG", "TRACY_ENABLE", "TRACY_ON_DEMAND", "TRACY_NO_EXIT " }
		runtime "Debug"
		symbols "on"
		optimize "off"

	filter "configurations:Release"
		defines { "NDEBUG", "TRACY_ENABLE", "TRACY_ON_DEMAND" }
		runtime "Release"
		symbols "on"
		optimize "speed"

	filter "configurations:Dist"
		defines { "NDEBUG", "TRACY_ON_DEMAND", "TRACY_NO_CALLSTACK" }
		runtime "Release"
		symbols "off"
		optimize "full"