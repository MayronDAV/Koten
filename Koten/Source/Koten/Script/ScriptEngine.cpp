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

		static void PrintAssemblyTypes(MonoAssembly* p_Assembly)
		{
			MonoImage* image = mono_assembly_get_image(p_Assembly);
			const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
			int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

			for (int32_t i = 0; i < numTypes; i++)
			{
				uint32_t cols[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

				const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
				const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
				KTN_CORE_TRACE("{}.{}", nameSpace, name);
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

		ScriptGlue::RegisterFunctions();

		// Retrieve and instantiate class (with constructor)
		s_Data->EntityClass = ScriptClass("KTN", "Entity");
		MonoObject* instance = s_Data->EntityClass.Instantiate();

		MonoMethod* printMessageFunc = s_Data->EntityClass.GetMethod("PrintMessage", 0);
		s_Data->EntityClass.InvokeMethod(instance, printMessageFunc);

		MonoMethod* printIntFunc = s_Data->EntityClass.GetMethod("PrintInt", 1);

		int value = 5;
		void* param = &value;

		s_Data->EntityClass.InvokeMethod(instance, printIntFunc, &param);

		MonoMethod* printIntsFunc = s_Data->EntityClass.GetMethod("PrintInts", 2);
		int value2 = 508;
		void* params[2] =
		{
			&value,
			&value2
		};
		s_Data->EntityClass.InvokeMethod(instance, printIntsFunc, params);

		MonoString* monoString = mono_string_new(s_Data->AppDomain, "Hello World from C++!");
		MonoMethod* printCustomMessageFunc = s_Data->EntityClass.GetMethod("PrintCustomMessage", 1);
		void* stringParam = monoString;
		s_Data->EntityClass.InvokeMethod(instance, printCustomMessageFunc, &stringParam);
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

} // namespace KTN
