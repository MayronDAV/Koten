#include "ktnpch.h"
#include "ScriptEngine.h"
#include "Koten/Core/FileSystem.h"

// lib
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include <mono/metadata/tabledefs.h>
#include <mono/metadata/mono-debug.h>



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
		};

		static ScriptEngineData* s_Data = nullptr;

		static void InitMono()
		{
			KTN_PROFILE_FUNCTION();

			mono_set_assemblies_path("Mono/lib");

			MonoDomain* rootDomain = mono_jit_init("KotenJITRuntime");
			KTN_CORE_ASSERT(rootDomain);

			s_Data->RootDomain = rootDomain;

			s_Data->AppDomain = mono_domain_create_appdomain((char*)"KotenScriptRuntime", nullptr);
			mono_domain_set(s_Data->AppDomain, true);
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

	} // namespace

	void ScriptEngine::Init()
	{
		KTN_PROFILE_FUNCTION();

		s_Data = new ScriptEngineData();

		InitMono();

		s_Data->CoreAssembly = LoadMonoAssembly("Resources/Scripts/Koten-ScriptCore.dll");
		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);
		PrintAssemblyTypes(s_Data->CoreAssembly);

		MonoClass* monoClass = mono_class_from_name(s_Data->CoreAssemblyImage, "KTN", "Main");
		MonoObject* instance = mono_object_new(s_Data->AppDomain, monoClass);
		mono_runtime_object_init(instance);

		{
			MonoMethod* method = mono_class_get_method_from_name(monoClass, "PrintMessage", -1);
			MonoObject* exception = nullptr;
			mono_runtime_invoke(method, instance, nullptr, &exception);
		}

		{
			MonoMethod* method = mono_class_get_method_from_name(monoClass, "PrintIntMessage", -1);
			MonoObject* exception = nullptr;
			int value = 3;
			void* params = &value;
			mono_runtime_invoke(method, instance, &params, &exception);
		}

		{
			MonoMethod* method = mono_class_get_method_from_name(monoClass, "PrintCustomMessage", -1);
			MonoObject* exception = nullptr;
			MonoString* str = mono_string_new(s_Data->AppDomain, "Hello World from C++");
			void* params = str;
			mono_runtime_invoke(method, instance, &params, &exception);
		}
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

} // namespace KTN
