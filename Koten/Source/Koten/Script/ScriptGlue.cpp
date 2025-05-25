#include "ktnpch.h"
#include "ScriptGlue.h"
#include "ScriptEngine.h"
#include "Koten/OS/KeyCodes.h"
#include "Koten/OS/MouseCodes.h"
#include "Koten/OS/Input.h"

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

		KTN_ADD_INTERNAL_CALL(Entity_HasComponent);
		KTN_ADD_INTERNAL_CALL(TransformComponent_GetLocalTranslation);
		KTN_ADD_INTERNAL_CALL(TransformComponent_SetLocalTranslation);

		KTN_ADD_INTERNAL_CALL(Input_IsKeyPressed);
	}

} // namespace KTN