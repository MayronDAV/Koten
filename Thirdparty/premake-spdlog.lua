project "spdlog"
	kind "StaticLib"
	language "C++"
	cppdialect "C++11"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/Thirdparty/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/Thirdparty/%{prj.name}")

	defines "SPDLOG_COMPILED_LIB"

	files
	{
		"spdlog/include/**.h",
		"spdlog/src/**.cpp"
	}

	includedirs
	{
		"spdlog/include/"
	}
	
	filter "system:windows"
		defines { "UNICODE", "_UNICODE" }
		buildoptions { "/utf-8" }
		systemversion "latest"

	filter "system:linux"
		pic "on"
		systemversion "latest"
		buildoptions { "-finput-charset=UTF-8", "-fexec-charset=UTF-8" }

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