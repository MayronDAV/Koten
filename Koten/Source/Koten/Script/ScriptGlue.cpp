#include "ktnpch.h"
#include "ScriptGlue.h"
#include "ScriptEngine.h"
#include "Koten/OS/KeyCodes.h"
#include "Koten/OS/MouseCodes.h"
#include "Koten/OS/Input.h"
#include "Koten/OS/GamepadCodes.h"

// lib
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>



namespace KTN
{
	namespace
	{
		static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;

		#define KTN_ADD_INTERNAL_CALL(Name) mono_add_internal_call("KTN.InternalCalls::" #Name, Name)

		static float Time_GetTime()
		{
			return (float)Time::GetTime();
		}

		static float Time_GetDeltaTime()
		{
			return (float)Time::GetDeltaTime();
		}

		static MonoObject* GetScriptInstance(UUID p_UUID)
		{
			return ScriptEngine::GetManagedInstance(p_UUID);
		}

		static bool Entity_HasComponent(UUID p_EntityID, MonoReflectionType* p_ComponentType)
		{
			Scene* scene = ScriptEngine::GetSceneContext();
			KTN_CORE_ASSERT(scene);
			Entity entity = scene->GetEntityByUUID(p_EntityID);
			KTN_CORE_ASSERT(entity);

			MonoType* managedType = mono_reflection_type_get_type(p_ComponentType);
			KTN_CORE_ASSERT(s_EntityHasComponentFuncs.find(managedType) != s_EntityHasComponentFuncs.end());
			return s_EntityHasComponentFuncs.at(managedType)(entity);
		}

		static uint64_t Entity_GetEntityByTag(MonoString* p_Tag)
		{
			char* tagCStr = mono_string_to_utf8(p_Tag);

			Scene* scene = ScriptEngine::GetSceneContext();
			KTN_CORE_ASSERT(scene);
			Entity entity = scene->GetEntityByTag(tagCStr);
			mono_free(tagCStr);
			if (!entity)
				return 0;

			return entity.GetUUID();
		}

		static void TransformComponent_GetLocalTranslation(UUID p_EntityID, glm::vec3* p_Result)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Scene* scene = ScriptEngine::GetSceneContext();
			KTN_CORE_ASSERT(scene);
			Entity entity = scene->GetEntityByUUID(p_EntityID);
			KTN_CORE_ASSERT(entity);

			*p_Result = entity.GetComponent<TransformComponent>().GetLocalTranslation();
		}

		static void TransformComponent_SetLocalTranslation(UUID p_EntityID, glm::vec3* p_Value)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Scene* scene = ScriptEngine::GetSceneContext();
			KTN_CORE_ASSERT(scene);
			Entity entity = scene->GetEntityByUUID(p_EntityID);
			KTN_CORE_ASSERT(entity);

			entity.GetComponent<TransformComponent>().SetLocalTranslation(*p_Value);
		}

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

		template <typename... Component>
		static void RegisterComponent()
		{
			KTN_PROFILE_FUNCTION_LOW();

			([]()
			{
				std::string_view typeName = typeid(Component).name();
				size_t pos = typeName.find_last_of(':');
				std::string_view structName = typeName.substr(pos + 1);
				std::string managedTypename = std::string("KTN.") + std::string(structName.data());

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

		KTN_ADD_INTERNAL_CALL(Time_GetTime);
		KTN_ADD_INTERNAL_CALL(Time_GetDeltaTime);

		KTN_ADD_INTERNAL_CALL(GetScriptInstance);
		KTN_ADD_INTERNAL_CALL(Entity_HasComponent);
		KTN_ADD_INTERNAL_CALL(Entity_GetEntityByTag);

		KTN_ADD_INTERNAL_CALL(TransformComponent_GetLocalTranslation);
		KTN_ADD_INTERNAL_CALL(TransformComponent_SetLocalTranslation);

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