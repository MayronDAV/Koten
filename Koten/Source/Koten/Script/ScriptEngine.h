#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Scene/Scene.h"
#include "Koten/Scene/Entity.h"
#include "Utils.h"


extern "C" {
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoAssembly MonoAssembly;
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoClassField MonoClassField;
	typedef struct _MonoString MonoString;
	typedef struct _MonoDomain MonoDomain;
}

namespace KTN
{
	struct ScriptField
	{
		ScriptFieldType Type;
		std::string Name;
		bool IsPrivate = false;

		bool ShowInEditor = false;
		bool Serialize = false;

		MonoClassField* ClassField;
	};

	struct ScriptFieldInstance
	{
		ScriptField Field;

		ScriptFieldInstance()
		{
			memset(m_Buffer, 0, sizeof(m_Buffer));
		}

		template<typename T>
		T GetValue()
		{
			static_assert(sizeof(T) <= 16, "Type too large!");
			return *(T*)m_Buffer;
		}

		template<typename T>
		void SetValue(T p_Value)
		{
			static_assert(sizeof(T) <= 16, "Type too large!");
			m_Changed = true;
			memcpy(m_Buffer, &p_Value, sizeof(T));
		}
	
	private:
		bool m_Changed = false;
		uint8_t m_Buffer[16];

		friend class ScriptEngine;
		friend class ScriptInstance;
	};

	using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;

	class KTN_API ScriptClass
	{
		public:
			ScriptClass() = default;
			ScriptClass(const std::string& p_Namespace, const std::string& p_Name, bool p_IsCore = false);

			MonoObject* Instantiate();
			MonoMethod* GetMethod(const std::string& p_Name, int p_ParameterCount);
			MonoObject* InvokeMethod(MonoObject* p_Instance, MonoMethod* p_Method, void** p_Params = nullptr);

			std::map<std::string, ScriptField>& GetFields() { return m_Fields; }
			const std::map<std::string, ScriptField>& GetFields() const { return m_Fields; }

		private:
			std::string m_Namespace = "";
			std::string m_Name = "";

			std::map<std::string, ScriptField> m_Fields;

			MonoClass* m_Class = nullptr;
	};

	class KTN_API ScriptInstance
	{
		public:
			ScriptInstance(Ref<ScriptClass> p_ScriptClass, Entity p_Entity);

			void InvokeOnCreate();
			void InvokeOnUpdate();
			void TryInvokeMethod(const std::string& p_MethodName, int p_Count = 0, void** p_Params = nullptr);
			void TryInvokeParentMethod(const std::string& p_MethodName, int p_Count = 0, void** p_Params = nullptr);

			Ref<ScriptClass> GetScriptClass() { return m_ScriptClass; }

			template<typename T>
			T GetFieldValue(const std::string& p_Name)
			{
				KTN_PROFILE_FUNCTION();

				static_assert(sizeof(T) <= 16, "Type too large!");

				static char fieldValueBuffer[16];
				bool success = GetFieldValueInternal(p_Name, fieldValueBuffer);
				if (!success)
					return T();

				return *(T*)fieldValueBuffer;
			}

			template<typename T>
			void SetFieldValue(const std::string& p_Name, T p_Value)
			{
				KTN_PROFILE_FUNCTION();

				static_assert(sizeof(T) <= 16, "Type too large!");

				SetFieldValueInternal(p_Name, &p_Value);
			}

			MonoObject* GetManagedObject() { return m_Instance; }

		private:
			bool GetFieldValueInternal(const std::string& p_Name, void* p_Buffer);
			bool SetFieldValueInternal(const std::string& p_Name, const	void* p_Value);

		private:
			Ref<ScriptClass> m_ScriptClass;

			MonoObject* m_Instance = nullptr;
			MonoMethod* m_Constructor = nullptr;
			MonoMethod* m_OnCreateMethod = nullptr;
			MonoMethod* m_OnUpdateMethod = nullptr;

			friend class ScriptEngine;
	};

	class KTN_API ScriptEngine
	{
		public:
			static void Init();
			static void Shutdown();

			static bool LoadAssembly(const std::string& p_Path);
			static bool LoadAppAssembly(const std::string& p_Path);
			static bool CompileLoadAppAssembly(bool p_ForceCompile = false);
			static bool CompileScripts(const std::filesystem::path& p_SourcePath, const std::filesystem::path& p_OutFolder);

			static void OnRuntimeStart(Scene* p_Scene);
			static void OnRuntimeUpdate(Scene* p_Scene);
			static void OnRuntimeStop();

			static void ReloadAssembly();
			static void RecompileAppAssembly();

			static bool EntityClassExists(const std::string& p_FullClassName);
			static void OnCreateEntity(Entity p_Entity);
			static void OnUpdateEntity(Entity p_Entity);

			static MonoString* CreateString(const char* p_String);

			static Ref<ScriptClass> GetEntityClass(const std::string& p_Name);
			static Ref<ScriptInstance> GetEntityScriptInstance(UUID p_EntityID);
			static ScriptFieldMap& GetScriptFieldMap(Entity p_Entity);
			static const std::unordered_map<std::string, Ref<ScriptClass>>& GetEntityClasses();
			static const std::unordered_map<UUID, Ref<ScriptInstance>>& GetEntityInstances();
			static MonoImage* GetCoreAssemblyImage();
			static MonoDomain* GetAppDomain();

			static MonoObject* GetManagedInstance(UUID p_UUID);
	};

} // namespace KTN