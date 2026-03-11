include "./Tools/Customization/solution_items.lua"
include "premake-dependencies.lua"


workspace "Koten"
    architecture "x64"
    startproject "Editor"
    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
    

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

    solution_items
    {
        ".editorconfig"
    }

    IMGUI_GLFW 		= "ON"
    IMGUI_OPENGL 	= "ON"

    filter "system:windows"
        systemversion "latest"
        multiprocessorcompile ("On")
    
        filter { "system:windows", "configurations:Dist" }
            linkoptions { "/SUBSYSTEM:WINDOWS" }

    filter "configurations:Debug"
        debugenvs
        {
            "MONO_LOG_LEVEL=debug",
            "MONO_LOG_MASK=all"
        }

    filter "configurations:Release"
        debugenvs
        {
            "MONO_LOG_LEVEL=info",
            "MONO_LOG_MASK=all"
        }

    filter "configurations:Dist"
        debugenvs
        {
            "MONO_LOG_LEVEL=error"
        }
    
    
group "Thirdparty"
    include "Tools"
    include "Thirdparty/premake-imgui.lua"
    include "Thirdparty/premake-spdlog.lua"
    include "Thirdparty/premake-yaml-cpp.lua"
    include "Thirdparty/ImGuizmo"
    include "Koten/include-dependencies.lua"
group ""

group "Core"
    include "Koten"
    include "Koten-ScriptCore"
group ""

group "Tools"
    include "Editor"
    include "Runtime"
group ""
