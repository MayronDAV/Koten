project "Koten-ScriptCore"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"

	targetdir ("../Editor/Resources/Scripts")
	objdir ("../Editor/Resources/Scripts/Intermediates")
	
	files
	{
		"Source/**.cs"
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