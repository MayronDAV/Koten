#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Scene/Scene.h"
#include "Koten/Scene/Entity.h"


extern "C" {
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoAssembly MonoAssembly;
	typedef struct _MonoImage MonoImage;
}

namespace KTN
{
	class KTN_API ScriptClass
	{
		public:
			ScriptClass() = default;
			ScriptClass(const std::string& p_Namespace, const std::string& p_Name);

			MonoObject* Instantiate();
			MonoMethod* GetMethod(const std::string& p_Name, int p_ParameterCount);
			MonoObject* InvokeMethod(MonoObject* p_Instance, MonoMethod* p_Method, void** p_Params = nullptr);

		private:
			std::string m_Namespace = "";
			std::string m_Name = "";

			MonoClass* m_Class = nullptr;
	};

	class KTN_API ScriptInstance
	{
		public:
			ScriptInstance(Ref<ScriptClass> p_ScriptClass, Entity p_Entity);

			void InvokeOnCreate();
			void InvokeOnUpdate();

		private:
			Ref<ScriptClass> m_ScriptClass;

			MonoObject* m_Instance = nullptr;
			MonoMethod* m_Constructor = nullptr;
			MonoMethod* m_OnCreateMethod = nullptr;
			MonoMethod* m_OnUpdateMethod = nullptr;
	};

	class KTN_API ScriptEngine
	{
		public:
			static void Init();
			static void Shutdown();

			static bool LoadAssembly(const std::string& p_Path);

			static void OnRuntimeStart(Scene* p_Scene);
			static void OnRuntimeUpdate(Scene* p_Scene);
			static void OnRuntimeStop();

			static bool EntityClassExists(const std::string& p_FullClassName);
			static void OnCreateEntity(Entity p_Entity);
			static void OnUpdateEntity(Entity p_Entity);

			static const std::unordered_map<std::string, Ref<ScriptClass>>& GetEntityClasses();
			static const std::unordered_map<UUID, Ref<ScriptInstance>>& GetEntityInstances();
			static MonoImage* GetCoreAssemblyImage();

			// TEMPORARY: REMOVE WHEN WE HAVE A SCENE MANAGER
			static Scene* GetSceneContext();
	};

} // namespace KTN