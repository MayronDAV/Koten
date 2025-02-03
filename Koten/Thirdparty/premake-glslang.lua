include "glslang-gen_build_info.lua"

project "glslang"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/Thirdparty/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/Thirdparty/%{prj.name}")

	files
	{
		"glslang/glslang/ResourceLimits/**.cpp",
		"glslang/glslang/Public/**.h",
		"glslang/glslang/Public/**.cpp",
		"glslang/glslang/OSDependent/*.h",
		"glslang/glslang/MachineIndependent/**.h",
		"glslang/glslang/MachineIndependent/**.cpp",
		"glslang/glslang/Include/**.h",
		"glslang/glslang/GenericCodeGen/**.cpp",
		"glslang/glslang/ExtensionHeaders/*.glsl",
		"glslang/SPIRV/*.h",
		"glslang/SPIRV/*.cpp",
	}

	includedirs
	{
		"glslang",
		"glslang/glslang",
		"glslang/SPIRV"
	}

	generate_build_info()

	filter "system:linux"
		pic "on"
		systemversion "latest"
		files { "glslang/glslang/OSDependent/Unix/**.cpp" }

	filter "system:windows"
		systemversion "latest"
		files { "glslang/glslang/OSDependent/Windows/**.cpp" }

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