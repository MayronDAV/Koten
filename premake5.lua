include "premake-dependencies.lua"


workspace "Koten"
	architecture "x64"
	startproject "Sandbox"
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
	
	
group "Thirdparty"
	include "Tools"
	include "Thirdparty/premake-imgui.lua"
	include "Thirdparty/premake-spdlog.lua"
	include "Thirdparty/premake-yaml-cpp.lua"
group ""

group "Core"
	include "Koten"
group ""

group "Misc"
	include "Sandbox"
group ""
