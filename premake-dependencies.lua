IncludeDir					    = {}

-- PUBLIC

IncludeDir["ktn"] 			    = "%{wks.location}/Koten/Source/"
IncludeDir["entt"] 			    = "%{wks.location}/Thirdparty/entt/include/"
IncludeDir["glm"] 			    = "%{wks.location}/Thirdparty/glm/"
IncludeDir["imgui"] 		    = "%{wks.location}/Thirdparty/imgui/"
IncludeDir["spdlog"] 		    = "%{wks.location}/Thirdparty/spdlog/include/"
IncludeDir["yamlcpp"]		    = "%{wks.location}/Thirdparty/yaml-cpp/include"
IncludeDir["ImGuizmo"]		    = "%{wks.location}/Thirdparty/ImGuizmo"
IncludeDir["optick"]		    = "%{wks.location}/Koten/Thirdparty/Optick/src"

-- PRIVATE

IncludeDir["glad"] 			    = "%{wks.location}/Koten/Thirdparty/glad/include/"
IncludeDir["glfw"] 			    = "%{wks.location}/Koten/Thirdparty/glfw/include/"
IncludeDir["glslang"] 		    = "%{wks.location}/Koten/Thirdparty/glslang/"
IncludeDir["SPIRVCross"] 	    = "%{wks.location}/Koten/Thirdparty/SPIRV-Cross/"
IncludeDir["stb"] 			    = "%{wks.location}/Koten/Thirdparty/stb/"
IncludeDir["box2d"] 		    = "%{wks.location}/Koten/Thirdparty/box2d"
IncludeDir["mono"] 			    = "%{wks.location}/Koten/Thirdparty/mono/include"
IncludeDir["filewatch"] 	    = "%{wks.location}/Koten/Thirdparty/filewatch/include"
IncludeDir["msdfgen"] 	        = "%{wks.location}/Koten/Thirdparty/msdf-atlas-gen/msdfgen"
IncludeDir["msdf_atlas_gen"] 	= "%{wks.location}/Koten/Thirdparty/msdf-atlas-gen/msdf-atlas-gen"
IncludeDir["magic_enum"] 	    = "%{wks.location}/Koten/Thirdparty/magic_enum/include"

LibraryDir = {}
LibraryDir["mono"] 			    = "%{wks.location}/Koten/Thirdparty/mono/lib"

Library = {}
Library["mono"] 			    = "%{LibraryDir.mono}/libmono-static-sgen.lib"

-- Windows
Library["WinSock"] 			    = "Ws2_32.lib"
Library["WinMM"] 			    = "Winmm.lib"
Library["WinVersion"] 		    = "Version.lib"
Library["BCrypt"] 			    = "Bcrypt.lib"