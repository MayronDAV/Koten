#include "ktnpch.h"
#include "ScriptEngine.h"
#include "Koten/Core/FileSystem.h"

#include "ScriptGlue.h"

// lib
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include <mono/metadata/tabledefs.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/exception.h>



namespace KTN
{
	namespace
	{
		struct ScriptEngineData
		{
			MonoDomain* RootDomain = nullptr;
			MonoDomain* AppDomain = nullptr;

			MonoAssembly* CoreAssembly = nullptr;
			MonoImage* CoreAssemblyImage = nullptr;

			std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
			std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;

			Scene* SceneContext = nullptr;

			ScriptClass EntityClass;
		};

		static ScriptEngineData* s_Data = nullptr;

		static void InitMono()
		{
			KTN_PROFILE_FUNCTION();

			mono_set_assemblies_path("Mono/lib");

			MonoDomain* rootDomain = mono_jit_init("KotenJITRuntime");
			KTN_CORE_ASSERT(rootDomain);

			s_Data->RootDomain = rootDomain;
		}

		static void ShutdownMono()
		{
			KTN_PROFILE_FUNCTION();

			if (s_Data->AppDomain)
			{
				mono_domain_set(mono_get_root_domain(), false);
				mono_domain_unload(s_Data->AppDomain);
				s_Data->AppDomain = nullptr;
			}

			if (s_Data->RootDomain)
			{
				mono_jit_cleanup(s_Data->RootDomain);
				s_Data->RootDomain = nullptr;
			}
		}

		static MonoAssembly* LoadMonoAssembly(const std::string& p_AssemblyPath)
		{
			ScopedBuffer buffer = FileSystem::ReadFileBinary(p_AssemblyPath);

			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(buffer.As<char>(), (uint32_t)buffer.Size(), 1, &status, 0);

			if (status != MONO_IMAGE_OK)
			{
				const char* errorMessage = mono_image_strerror(status);
				KTN_CORE_ERROR("Failed to load assembly: {}. Error: {}", p_AssemblyPath, errorMessage);
				return nullptr;
			}

			MonoAssembly* assembly = mono_assembly_load_from_full(image, p_AssemblyPath.c_str(), &status, 0);
			mono_image_close(image);
			return assembly;
		}

		static void LoadAssemblyClasses(MonoAssembly* p_Assembly)
		{
			s_Data->EntityClasses.clear();

			MonoImage* image = mono_assembly_get_image(p_Assembly);
			const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
			int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
			MonoClass* entityClass = mono_class_from_name(image, "KTN", "Entity");

			for (int32_t i = 0; i < numTypes; i++)
			{
				uint32_t cols[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

				const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
				const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
				std::string fullName;
				if (strlen(nameSpace) != 0)
					fullName = fmt::format("{}.{}", nameSpace, name);
				else
					fullName = name;

				MonoClass* monoClass = mono_class_from_name(image, nameSpace, name);

				if (monoClass == entityClass)
					continue;

				bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false);
				if (isEntity)
					s_Data->EntityClasses[fullName] = CreateRef<ScriptClass>(nameSpace, name);
			}
		}

		MonoObject* InstantiateClass(MonoClass* p_Class)
		{
			MonoObject* instance = mono_object_new(s_Data->AppDomain, p_Class);
			mono_runtime_object_init(instance);
			return instance;
		}

	} // namespace

	#pragma region ScriptEngine

	void ScriptEngine::Init()
	{
		KTN_PROFILE_FUNCTION();

		s_Data = new ScriptEngineData();

		InitMono();

		bool status = LoadAssembly("Resources/Scripts/Koten-ScriptCore.dll");
		if (!status)
		{
			KTN_CORE_ERROR("[ScriptEngine] Could not load Koten-ScriptCore assembly.");
			return;
		}
		LoadAssemblyClasses(s_Data->CoreAssembly);

		ScriptGlue::RegisterComponents();
		ScriptGlue::RegisterFunctions();

		s_Data->EntityClass = ScriptClass("KTN", "Entity");
	}

	void ScriptEngine::Shutdown()
	{
		KTN_PROFILE_FUNCTION();

		ShutdownMono();

		if (s_Data)
		{
			delete s_Data;
			s_Data = nullptr;
		}
	}

	bool ScriptEngine::LoadAssembly(const std::string& p_Path)
	{
		KTN_PROFILE_FUNCTION();

		s_Data->AppDomain = mono_domain_create_appdomain((char*)"KotenScriptRuntime", nullptr);
		mono_domain_set(s_Data->AppDomain, true);

		s_Data->CoreAssembly = LoadMonoAssembly(p_Path);
		if (!s_Data->CoreAssembly)
		{
			KTN_CORE_ERROR("Failed to load assembly: {}", p_Path);
			return false;
		}

		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);
		return true;
	}

	void ScriptEngine::OnRuntimeStart(Scene* p_Scene)
	{
		KTN_PROFILE_FUNCTION();

		s_Data->SceneContext = p_Scene;

		p_Scene->GetRegistry().view<TagComponent, ScriptComponent>().each(
		[&](auto p_Entt, TagComponent& p_Tag, ScriptComponent& p_Sc)
		{
			auto entity = Entity(p_Entt, p_Scene);
			OnCreateEntity(entity);
		});
	}

	void ScriptEngine::OnRuntimeUpdate(Scene* p_Scene)
	{
		KTN_PROFILE_FUNCTION();

		p_Scene->GetRegistry().view<TagComponent, ScriptComponent>().each(
		[&](auto p_Entt, TagComponent& p_Tag, ScriptComponent& p_Sc)
		{
			auto entity = Entity(p_Entt, p_Scene);
			OnUpdateEntity(entity);
		});
	}

	void ScriptEngine::OnRuntimeStop()
	{
		KTN_PROFILE_FUNCTION();

		s_Data->SceneContext = nullptr;

		s_Data->EntityInstances.clear();
	}

	bool ScriptEngine::EntityClassExists(const std::string& p_FullClassName)
	{
		return s_Data->EntityClasses.find(p_FullClassName) != s_Data->EntityClasses.end();
	}

	void ScriptEngine::OnCreateEntity(Entity p_Entity)
	{
		KTN_PROFILE_FUNCTION();

		auto sc = p_Entity.TryGetComponent<ScriptComponent>();
		if (!sc)
			return;

		if (ScriptEngine::EntityClassExists(sc->FullClassName))
		{
			Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_Data->EntityClasses[sc->FullClassName], p_Entity);
			s_Data->EntityInstances[p_Entity.GetUUID()] = instance;
			instance->InvokeOnCreate();
		}
	}

	void ScriptEngine::OnUpdateEntity(Entity p_Entity)
	{
		KTN_PROFILE_FUNCTION();

		UUID entityUUID = p_Entity.GetUUID();
		if (s_Data->EntityInstances.find(entityUUID) == s_Data->EntityInstances.end())
		{
			KTN_CORE_ERROR("Script instance not found for entity: {}", (uint64_t)entityUUID);
			return;
		}

		Ref<ScriptInstance> instance = s_Data->EntityInstances[entityUUID];
		instance->InvokeOnUpdate();
	}

	const std::unordered_map<std::string, Ref<ScriptClass>>& ScriptEngine::GetEntityClasses()
	{
		return s_Data->EntityClasses;
	}

	const std::unordered_map<UUID, Ref<ScriptInstance>>& ScriptEngine::GetEntityInstances()
	{
		return s_Data->EntityInstances;
	}

	MonoImage* KTN::ScriptEngine::GetCoreAssemblyImage()
	{
		return s_Data->CoreAssemblyImage;
	}

	Scene* ScriptEngine::GetSceneContext()
	{
		return s_Data->SceneContext;
	}

	#pragma endregion

	#pragma region ScriptClass

	ScriptClass::ScriptClass(const std::string& p_Namespace, const std::string& p_Name)
		: m_Name(p_Name), m_Namespace(p_Namespace), m_Class(nullptr)
	{
		KTN_PROFILE_FUNCTION();

		m_Class = mono_class_from_name(s_Data->CoreAssemblyImage, p_Namespace.c_str(), p_Name.c_str());
	}

	MonoObject* ScriptClass::Instantiate()
	{
		KTN_PROFILE_FUNCTION();

		return InstantiateClass(m_Class);
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& p_Name, int p_ParameterCount)
	{
		return mono_class_get_method_from_name(m_Class, p_Name.c_str(), p_ParameterCount);
	}

	MonoObject* ScriptClass::InvokeMethod(MonoObject* p_Instance, MonoMethod* p_Method, void** p_Params)
	{
		MonoObject* exception = nullptr;
		return mono_runtime_invoke(p_Method, p_Instance, p_Params, &exception);
	}

	#pragma endregion

	#pragma region ScriptInstance

	ScriptInstance::ScriptInstance(Ref<ScriptClass> p_ScriptClass, Entity p_Entity)
		: m_ScriptClass(p_ScriptClass)
	{
		KTN_PROFILE_FUNCTION();

		m_Instance = p_ScriptClass->Instantiate();

		m_Constructor = s_Data->EntityClass.GetMethod(".ctor", 1);
		m_OnCreateMethod = p_ScriptClass->GetMethod("OnCreate", 0);
		m_OnUpdateMethod = p_ScriptClass->GetMethod("OnUpdate", 0);

		// Call Entity constructor
		{
			UUID entityID = p_Entity.GetUUID();
			void* param = &entityID;
			m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &param);
		}
	}

	void ScriptInstance::InvokeOnCreate()
	{
		KTN_PROFILE_FUNCTION();

		if (m_OnCreateMethod)
			m_ScriptClass->InvokeMethod(m_Instance, m_OnCreateMethod);
	}

	void ScriptInstance::InvokeOnUpdate()
	{
		KTN_PROFILE_FUNCTION();

		if (m_OnUpdateMethod)
			m_ScriptClass->InvokeMethod(m_Instance, m_OnUpdateMethod);
	}

	#pragma endregion

} // namespace KTN
