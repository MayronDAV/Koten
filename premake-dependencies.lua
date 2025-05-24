IncludeDir					= {}

-- PUBLIC

IncludeDir["ktn"] 			= "%{wks.location}/Koten/Source/"
IncludeDir["entt"] 			= "%{wks.location}/Thirdparty/entt/include/"
IncludeDir["glm"] 			= "%{wks.location}/Thirdparty/glm/"
IncludeDir["imgui"] 		= "%{wks.location}/Thirdparty/imgui/"
IncludeDir["spdlog"] 		= "%{wks.location}/Thirdparty/spdlog/include/"
IncludeDir["yamlcpp"]		= "%{wks.location}/Thirdparty/yaml-cpp/include"
IncludeDir["ImGuizmo"]		= "%{wks.location}/Thirdparty/ImGuizmo"
IncludeDir["optick"]		= "%{wks.location}/Koten/Thirdparty/Optick/src"

-- PRIVATE

IncludeDir["glad"] 			= "%{wks.location}/Koten/Thirdparty/glad/include/"
IncludeDir["glfw"] 			= "%{wks.location}/Koten/Thirdparty/glfw/include/"
IncludeDir["glslang"] 		= "%{wks.location}/Koten/Thirdparty/glslang/"
IncludeDir["SPIRVCross"] 	= "%{wks.location}/Koten/Thirdparty/SPIRV-Cross/"
IncludeDir["stb"] 			= "%{wks.location}/Koten/Thirdparty/stb/"
IncludeDir["box2d"] 		= "%{wks.location}/Koten/Thirdparty/box2d"
IncludeDir["mono"] 			= "%{wks.location}/Koten/Thirdparty/mono/include"

LibraryDir = {}
LibraryDir["mono"] 			= "%{wks.location}/Koten/Thirdparty/mono/lib"

Library = {}
Library["mono_windows"] 	= "%{LibraryDir.mono}/libmono-static-sgen.lib"
Library["mono_linux"] 		= "%{LibraryDir.mono}/libmonosgen-2.0.a"

-- Windows
Library["WinSock"] 			= "Ws2_32.lib"
Library["WinMM"] 			= "Winmm.lib"
Library["WinVersion"] 		= "Version.lib"
Library["BCrypt"] 			= "Bcrypt.lib"