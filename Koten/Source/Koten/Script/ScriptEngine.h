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
	typedef struct _MonoClassField MonoClassField;
}

namespace KTN
{
	enum class ScriptFieldType : uint8_t
	{
		None = 0,
		Float, Double,
		Bool, Char, Byte, Short, Int, Long,
		UByte, UShort, UInt, ULong,
		String,
		Vector2, Vector3, Vector4,
		Entity
	};

	struct ScriptField
	{
		ScriptFieldType Type;
		std::string Name;
		bool IsPrivate = false;

		MonoClassField* ClassField;
	};

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

			Ref<ScriptClass> GetScriptClass() { return m_ScriptClass; }

			template<typename T>
			T GetFieldValue(const std::string& p_Name)
			{
				static char fieldValueBuffer[8];
				bool success = GetFieldValueInternal(p_Name, fieldValueBuffer);
				if (!success)
					return T();

				return *(T*)fieldValueBuffer;
			}

			template<typename T>
			void SetFieldValue(const std::string& p_Name, const T& p_Value)
			{
				SetFieldValueInternal(p_Name, &p_Value);
			}

		private:
			bool GetFieldValueInternal(const std::string& p_Name, void* p_Buffer);
			bool SetFieldValueInternal(const std::string& p_Name, const	void* p_Value);

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
			static bool LoadAppAssembly(const std::string& p_Path);

			static void OnRuntimeStart(Scene* p_Scene);
			static void OnRuntimeUpdate(Scene* p_Scene);
			static void OnRuntimeStop();

			static bool EntityClassExists(const std::string& p_FullClassName);
			static void OnCreateEntity(Entity p_Entity);
			static void OnUpdateEntity(Entity p_Entity);

			static Ref<ScriptInstance> GetEntityScriptInstance(UUID p_EntityID);
			static const std::unordered_map<std::string, Ref<ScriptClass>>& GetEntityClasses();
			static const std::unordered_map<UUID, Ref<ScriptInstance>>& GetEntityInstances();
			static MonoImage* GetCoreAssemblyImage();

			// TEMPORARY: REMOVE WHEN WE HAVE A SCENE MANAGER
			static Scene* GetSceneContext();
	};

} // namespace KTN