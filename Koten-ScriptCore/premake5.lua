project "Koten-ScriptCore"
	kind "SharedLib"
	language "C#"

	targetdir ("%{wks.location}/Editor/Resources/Scripts")
	objdir ("%{wks.location}/Editor/Resources/Scripts/Intermediates")
	
	files
	{
		"Source/**.cs",
		"Properties/**.cs"
	}

	filter "system:windows"
		systemversion "latest"
		dotnetframework "4.7.2"

	filter "system:linux"
		systemversion "latest"
		toolset "mono"
		buildoptions {
            "--target:library",
            "-out:%{cfg.targetdir}/%{prj.name}.dll"
        }
	
	filter "configurations:Debug"
		optimize "off"
		symbols "default"

	filter "configurations:Release"
		optimize "on"
		symbols "default"

	filter "configurations:Dist"
		optimize "full"
		symbols "off"