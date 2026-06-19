project "Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	dependson "Koten"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")


	files
	{
		"Source/**.h",
		"Source/**.cpp"
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
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.imgui_node_editor}",
		"%{IncludeDir.optick}"
	}

	links
	{
		"Koten",
		"imgui",
		"spdlog",
		"yaml-cpp",
		"ImGuizmo",
		"imgui-node-editor"
	}

	defines { "KTN_PROFILE_ENABLED", "IMGUI_DEFINE_MATH_OPERATORS" }

	filter "system:windows"
		systemversion "latest"
		buildoptions { "/utf-8", "/Zc:char8_t-", "/wd4251" }
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
		buildoptions { "-finput-charset=UTF-8", "-fexec-charset=UTF-8", "-fno-char8_t", "-Wno-effc++" }
		linkoptions { "-Wl,-rpath,$$ORIGIN" } 
		defines "KTN_LINUX"

		postbuildcommands 
		{
			("{COPY} %{wks.location}/bin/" .. outputdir .. "/Koten/libKoten.so %{wks.location}/bin/" .. outputdir .. "/%{prj.name}/")
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