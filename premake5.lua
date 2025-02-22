include "premake-dependencies.lua"


workspace "Koten"
	architecture "x64"
	startproject "Editor"
	flags "MultiProcessorCompile"
	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
	
	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	IMGUI_GLFW 		= "ON"
	IMGUI_OPENGL 	= "ON"

	filter "system:windows"
		systemversion "latest"
	
		filter { "system:windows", "configurations:Dist" }
			linkoptions { "/SUBSYSTEM:WINDOWS" }
	
	
group "Thirdparty"
	include "Tools"
	include "Thirdparty/premake-imgui.lua"
	include "Thirdparty/premake-spdlog.lua"
	include "Thirdparty/premake-yaml-cpp.lua"
	include "Koten/include-dependencies.lua"
group ""

group "Core"
	include "Koten"
group ""

group "Tools"
	include "Editor"
group ""

group "Misc"
	include "Sandbox"
group ""
