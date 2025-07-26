#include "ktnpch.h"
#include "ScriptGlue.h"
#include "ScriptEngine.h"
#include "Koten/OS/KeyCodes.h"
#include "Koten/OS/MouseCodes.h"
#include "Koten/OS/Input.h"
#include "Koten/OS/GamepadCodes.h"
#include "Koten/Project/Project.h"
#include "Koten/Scene/SceneManager.h"

// lib
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>



namespace KTN
{
	namespace
	{
		static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;

		std::string MonoStringToString(MonoString* p_String)
		{
			char* cStr = mono_string_to_utf8(p_String);
			std::string str(cStr);
			mono_free(cStr);
			return str;
		}

		#define KTN_ADD_INTERNAL_CALL(Name) mono_add_internal_call("KTN.InternalCalls::" #Name, Name)

		static MonoObject* GetScriptInstance(UUID p_UUID)
		{
			return ScriptEngine::GetManagedInstance(p_UUID);
		}

		#pragma region Time
		static float Time_GetTime()
		{
			return (float)Time::GetTime();
		}

		static float Time_GetDeltaTime()
		{
			return (float)Time::GetDeltaTime();
		}
		#pragma endregion

		#pragma region Entity
		static bool Entity_HasComponent(UUID p_EntityID, MonoReflectionType* p_ComponentType)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);

			MonoType* managedType = mono_reflection_type_get_type(p_ComponentType);
			KTN_CORE_VERIFY(s_EntityHasComponentFuncs.find(managedType) != s_EntityHasComponentFuncs.end());
			return s_EntityHasComponentFuncs.at(managedType)(entity);
		}

		static uint64_t Entity_GetEntityByTag(MonoString* p_Tag)
		{
			KTN_PROFILE_FUNCTION_LOW();

			char* tagCStr = mono_string_to_utf8(p_Tag);
			Entity entity = SceneManager::GetEntityByTag(tagCStr);
			KTN_CORE_VERIFY(entity);
			mono_free(tagCStr);
			if (!entity)
				return 0;

			return entity.GetUUID();
		}

		static bool Entity_IsValid(UUID p_EntityID)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			return entity ? true : false;
		}

		#pragma	endregion

		#pragma region TransformComponent
		static void TransformComponent_GetLocalTranslation(UUID p_EntityID, glm::vec3* p_Result)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TransformComponent>());

			*p_Result = entity.GetComponent<TransformComponent>().GetLocalTranslation();
		}

		static void TransformComponent_SetLocalTranslation(UUID p_EntityID, glm::vec3* p_Value)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TransformComponent>());

			entity.GetComponent<TransformComponent>().SetLocalTranslation(*p_Value);
		}
		#pragma endregion

		#pragma region TextRendererComponent
		static MonoString* TextRendererComponent_GetString(UUID p_EntityID)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			return ScriptEngine::CreateString(tc.String.c_str());
		}

		static void TextRendererComponent_SetString(UUID p_EntityID, MonoString* p_String)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			tc.String = MonoStringToString(p_String);
		}

		static void TextRendererComponent_SetFont(UUID p_EntityID, MonoString* p_Path)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			std::string fontPath = MonoStringToString(p_Path);
			if (fontPath.empty())
				tc.Font = DFFont::GetDefault();
			else
			{
				tc.Font = AssetManager::Get()->GetHandleByPath(fontPath);
				KTN_CORE_VERIFY(tc.Font != 0, "Failed to load font from path: " + fontPath + ", Try importing the font first!");
			}
		}

		static MonoString* TextRendererComponent_GetFontPath(UUID p_EntityID)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			return ScriptEngine::CreateString(AssetManager::Get()->GetMetadata(tc.Font).FilePath.c_str());
		}

		static MonoString* TextRendererComponent_GetFontName(UUID p_EntityID)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			return ScriptEngine::CreateString(FileSystem::GetStem(AssetManager::Get()->GetMetadata(tc.Font).FilePath).c_str());
		}

		static void TextRendererComponent_GetColor(UUID p_EntityID, glm::vec4* p_Color)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			*p_Color = tc.Color;
		}

		static void TextRendererComponent_SetColor(UUID p_EntityID, glm::vec4* p_Color)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			tc.Color = *p_Color;
		}

		static void TextRendererComponent_GetBgColor(UUID p_EntityID, glm::vec4* p_Color)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			*p_Color = tc.BgColor;
		}

		static void TextRendererComponent_SetBgColor(UUID p_EntityID, glm::vec4* p_Color)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			tc.BgColor = *p_Color;
		}

		static void TextRendererComponent_GetCharBgColor(UUID p_EntityID, glm::vec4* p_Color)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			*p_Color = tc.CharBgColor;
		}

		static void TextRendererComponent_SetCharBgColor(UUID p_EntityID, glm::vec4* p_Color)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			tc.CharBgColor = *p_Color;
		}

		static bool TextRendererComponent_GetDrawBg(UUID p_EntityID)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			return tc.DrawBg;
		}

		static void TextRendererComponent_SetDrawBg(UUID p_EntityID, bool p_Enable)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			tc.DrawBg = p_Enable;
		}

		static float TextRendererComponent_GetKerning(UUID p_EntityID)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			return tc.Kerning;
		}

		static void TextRendererComponent_SetKerning(UUID p_EntityID, float p_Kerning)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			tc.Kerning = p_Kerning;
		}

		static float TextRendererComponent_GetLineSpacing(UUID p_EntityID)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			return tc.LineSpacing;
		}

		static void TextRendererComponent_SetLineSpacing(UUID p_EntityID, float p_LineSpacing)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			tc.LineSpacing = p_LineSpacing;
		}
		#pragma endregion

		#pragma region Input
		static bool Input_IsKeyPressed(KeyCode p_Keycode)
		{
			return Input::IsKeyPressed(p_Keycode);
		}

		static bool Input_IsKeyReleased(KeyCode p_Keycode)
		{
			return Input::IsKeyReleased(p_Keycode);
		}

		static bool Input_IsMouseButtonPressed(MouseCode p_MouseCode)
		{
			return Input::IsMouseButtonPressed(p_MouseCode);
		}

		static bool Input_IsMouseButtonReleased(MouseCode p_MouseCode)
		{
			return Input::IsMouseButtonReleased(p_MouseCode);
		}

		static bool Input_IsControllerButtonPressed(int p_ControllerIndex, GamepadCode p_Button)
		{
			return Input::IsControllerButtonPressed(p_ControllerIndex, p_Button);
		}

		static bool Input_IsControllerButtonReleased(int p_ControllerIndex, GamepadCode p_Button)
		{
			return Input::IsControllerButtonReleased(p_ControllerIndex, p_Button);
		}

		static bool Input_IsControllerButtonHeld(int p_ControllerIndex, GamepadCode p_Button)
		{
			return Input::IsControllerButtonHeld(p_ControllerIndex, p_Button);
		}

		static bool Input_IsControllerConnected(int p_ControllerIndex)
		{
			return Input::IsControllerPresent(p_ControllerIndex);
		}

		static float Input_GetControllerAxis(int p_ControllerIndex, GamepadAxisCode p_Axis)
		{
			return Input::GetControllerAxis(p_ControllerIndex, p_Axis);
		}

		static float Input_GetMouseX()
		{
			return Input::GetMouseX();
		}

		static float Input_GetMouseY()
		{
			return Input::GetMouseY();
		}

		static void Input_GetMousePosition(glm::vec2* p_Result)
		{
			*p_Result = Input::GetMousePosition();
		}

		static void Input_GetConnectedControllerIDs(MonoArray* p_Out, MonoException** p_Exception)
		{
			KTN_PROFILE_FUNCTION_LOW();

			auto data = Input::GetConnectedControllerIDs();
			for (size_t i = 0; i < data.size(); ++i) 
			{
				mono_array_set(p_Out, int, (int)i, data[i]);
			}
		}

		static MonoString* Input_GetControllerName(int p_ControllerIndex)
		{
			KTN_PROFILE_FUNCTION_LOW();
			KTN_CORE_ASSERT(Input::IsControllerPresent(p_ControllerIndex), "Controller is not connected!");

			std::string name = Input::GetController(p_ControllerIndex)->Name;
			return ScriptEngine::CreateString(name.c_str());
		}
		#pragma endregion

		std::string DemangleToKTNClassName(const std::string& p_MangledName) 
		{
			KTN_PROFILE_FUNCTION_LOW();

			auto n3pos = p_MangledName.find("N3");
			if (n3pos == std::string::npos) 
				return p_MangledName; // no need to demangle

			size_t currentPos = n3pos + 5;
			std::string result = "KTN.";

			while (currentPos < p_MangledName.length()) 
			{
				size_t numStart = currentPos;
				size_t numEnd = p_MangledName.find_first_not_of("0123456789", numStart);
				int segmentLength = std::stoi(p_MangledName.substr(numStart, numEnd - numStart));

				size_t pos = numEnd + segmentLength;
				std::string segment = p_MangledName.substr(numEnd, segmentLength);
				if (pos < p_MangledName.length() && p_MangledName[pos] == 'E')
					return result + segment;

				currentPos = pos;
			}

			size_t numStart = n3pos + 5;
			size_t numEnd = p_MangledName.find_first_not_of("0123456789", numStart);
			if (numEnd != std::string::npos)
			{
				int length = std::stoi(p_MangledName.substr(numStart, numEnd - numStart));
				return result + p_MangledName.substr(numEnd, length);
			}

			return p_MangledName;
		}

		template <typename... Component>
		static void RegisterComponent()
		{
			KTN_PROFILE_FUNCTION_LOW();

			([]()
			{
				std::string_view typeName = typeid(Component).name();
				size_t pos = typeName.find_last_of(':');
				std::string_view structName = typeName.substr(pos + 1);

				std::string managedTypename = DemangleToKTNClassName(std::string("KTN.") + std::string(structName.data()));

				MonoType* managedType = mono_reflection_type_from_name(managedTypename.data(), ScriptEngine::GetCoreAssemblyImage());
				if (!managedType)
				{
					KTN_CORE_ERROR("Could not find component type {}", managedTypename);
					return;
				}
				s_EntityHasComponentFuncs[managedType] = [](Entity p_Entity) { return p_Entity.HasComponent<Component>(); };
			}(), ...);
		}
	}

	void ScriptGlue::RegisterComponents()
	{
		KTN_PROFILE_FUNCTION();

		s_EntityHasComponentFuncs.clear();
		RegisterComponent<ALL_COMPONENTS>();
	}

	void ScriptGlue::RegisterFunctions()
	{
		KTN_PROFILE_FUNCTION();

		KTN_ADD_INTERNAL_CALL(GetScriptInstance);

		KTN_ADD_INTERNAL_CALL(Time_GetTime);
		KTN_ADD_INTERNAL_CALL(Time_GetDeltaTime);

		KTN_ADD_INTERNAL_CALL(Entity_HasComponent);
		KTN_ADD_INTERNAL_CALL(Entity_GetEntityByTag);
		KTN_ADD_INTERNAL_CALL(Entity_IsValid);

		KTN_ADD_INTERNAL_CALL(TransformComponent_GetLocalTranslation);
		KTN_ADD_INTERNAL_CALL(TransformComponent_SetLocalTranslation);

		KTN_ADD_INTERNAL_CALL(TextRendererComponent_GetString);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_SetString);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_SetFont);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_GetFontPath);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_GetFontName);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_GetColor);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_SetColor);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_GetBgColor);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_SetBgColor);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_GetCharBgColor);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_SetCharBgColor);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_GetDrawBg);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_SetDrawBg);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_GetKerning);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_SetKerning);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_GetLineSpacing);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_SetLineSpacing);

		KTN_ADD_INTERNAL_CALL(Input_IsKeyPressed);
		KTN_ADD_INTERNAL_CALL(Input_IsKeyReleased);
		KTN_ADD_INTERNAL_CALL(Input_IsMouseButtonPressed);
		KTN_ADD_INTERNAL_CALL(Input_IsMouseButtonReleased);
		KTN_ADD_INTERNAL_CALL(Input_IsControllerConnected);
		KTN_ADD_INTERNAL_CALL(Input_IsControllerButtonPressed);
		KTN_ADD_INTERNAL_CALL(Input_IsControllerButtonReleased);
		KTN_ADD_INTERNAL_CALL(Input_IsControllerButtonHeld);
		KTN_ADD_INTERNAL_CALL(Input_GetControllerAxis);
		KTN_ADD_INTERNAL_CALL(Input_GetConnectedControllerIDs);
		KTN_ADD_INTERNAL_CALL(Input_GetControllerName);
		KTN_ADD_INTERNAL_CALL(Input_GetMouseX);
		KTN_ADD_INTERNAL_CALL(Input_GetMouseY);
	}

} // namespace KTN