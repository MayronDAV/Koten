project "Koten"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "ktnpch.h"
	pchsource "Source/ktnpch.cpp"

	files
	{
		"Source/**.inl",

		"Source/ktnpch.h",
		"Source/ktnpch.cpp",

		"Source/Koten/**.h",
		"Source/Koten/**.cpp",

		"Source/Platform/GLFW/**.h",
		"Source/Platform/GLFW/**.cpp",
		"Source/Platform/OpenGL/**.h",
		"Source/Platform/OpenGL/**.cpp",

		"%{IncludeDir.optick}/**.h",
		"%{IncludeDir.optick}/**.cpp"
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
		"%{IncludeDir.msdfgen}",
		"%{IncludeDir.msdf_atlas_gen}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yamlcpp}",
		"%{IncludeDir.optick}",
		"%{IncludeDir.box2d}/include",
		"%{IncludeDir.box2d}/src",
		"%{IncludeDir.mono}",
		"%{IncludeDir.filewatch}",
		"%{IncludeDir.magic_enum}"
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
		"msdf-atlas-gen",
		"yaml-cpp",
		"box2d"
	}

	defines { "KTN_EXPORT", "OPTICK_EXPORT", "KTN_PROFILE_ENABLED", "USE_OPTICK=1", "MSDFGEN_PUBLIC=" }

	filter "action:vs*"
		buildoptions { "/Zc:preprocessor" }


	filter "files:Thirdparty/Optick/src/**.cpp"
		flags { "NoPCH" }

	filter "system:windows"
		systemversion "latest"
		buildoptions { "/utf-8", "/Zc:char8_t-", "/wd4251", "/wd4275" }

		links
		{
			"opengl32.lib",
			"%{Library.mono}",
			"%{Library.WinSock}",
			"%{Library.WinMM}",
			"%{Library.WinVersion}",
			"%{Library.BCrypt}"
		}

		defines
		{
			"KTN_WINDOWS",
			"GLFW_INCLUDE_NONE",
			"UNICODE", 
			"_UNICODE" 
		}

		files
		{
			"Source/Platform/Windows/**.h",
			"Source/Platform/Windows/**.cpp"
		}
	
	filter "system:linux"
        pic "on"
		links 
		{ 
			"GL",
			"monosgen-2.0",
			"freetype",
			"msdfgen"
		}
		libdirs { "%{LibraryDir.mono}" }
		buildoptions { "`pkg-config --cflags gtk+-3.0`", "-finput-charset=UTF-8", "-fexec-charset=UTF-8", "-fno-char8_t", "-Wno-effc++", "-fpermissive" }
		linkoptions { "`pkg-config --libs gtk+-3.0`" }
		defines
		{
			"KTN_LINUX",
			"GLFW_INCLUDE_NONE"
		}

		files
		{
			"Source/Platform/GTK/**.h",
			"Source/Platform/GTK/**.cpp"
		}

	filter "configurations:Debug"
		defines "KTN_DEBUG"
		runtime "Debug"
		symbols "on"
		optimize "off"

	filter "configurations:Release"
		defines "KTN_RELEASE"
		runtime "Release"
		symbols "on"
		optimize "speed"

	filter "configurations:Dist"
		defines "KTN_DIST"
		runtime "Release"
		symbols "off"
		optimize "full"
