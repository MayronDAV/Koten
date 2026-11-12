#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Scene/Scene.h"
#include "Koten/Scene/Entity.h"
#include "Utils.h"


extern "C" {
    typedef struct _MonoClass MonoClass;
    typedef struct _MonoType MonoType;
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
    using ScriptFieldValue = std::variant<
        std::monostate,
        float,
        double,
        bool,
        char16_t,
        int8_t,
        int16_t,
        int32_t,
        int64_t,
        uint8_t,
        uint16_t,
        uint32_t,
        uint64_t,
        glm::vec2,
        glm::vec3,
        glm::vec4,
        std::string,
        ObjectHandle
    >;

    enum class ScriptFieldCategory
    {
        Unsupported = 0,
        Primitive,
        Array,
        Enum,
        Struct,
        Class
    };

    struct ScriptField
    {
        ScriptFieldType Type;
        ScriptFieldCategory Category;

        std::string Name;

        bool IsPrivate    = false;
        bool ShowInEditor = false;
        bool Serialize    = false;

        MonoClassField* Field  = nullptr;
        MonoType* FieldType    = nullptr;
        MonoClass* FieldClass  = nullptr;
    };

    struct ScriptFieldInstance
    {
        ScriptField Field;
        ScriptFieldValue Value;

        bool IsChanged = false;

        template<typename T>
        bool IsType() const
        {
            return std::holds_alternative<T>(Value);
        }

        template<typename T>
        const T* GetIfValue() const
        {
            return std::get_if<T>(&Value);
        }

        template<typename T>
        T GetValue() const
        {
            if (!IsType<T>())
                return T();

            return std::get<T>(Value);
        }

        template<typename T>
        void SetValue(const T& value)
        {
            Value = value;
            IsChanged = true;
        }
    };

    class KTN_API ScriptFieldUtils
    {
        public:
            static bool SetFieldValue(MonoObject* p_Instance, const ScriptField& p_Field, const ScriptFieldValue& p_Value);
            static bool GetFieldValue(MonoObject* p_Instance, const ScriptField& p_Field, ScriptFieldValue& p_OutValue);
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

            std::string GetName() const { return m_Name; }
            std::string GetNamespace() const { return m_Namespace; }
            std::string GetFullName() const { return m_Namespace + "." + m_Name; }

            MonoClass* GetMonoClass() { return m_Class; }

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
            ~ScriptInstance();

            void InvokeOnCreate();
            void InvokeOnUpdate();
            void TryInvokeMethod(const std::string& p_MethodName, int p_Count = 0, void** p_Params = nullptr);
            void TryInvokeParentMethod(const std::string& p_MethodName, int p_Count = 0, void** p_Params = nullptr);

            Ref<ScriptClass> GetScriptClass() { return m_ScriptClass; }

            template<typename T>
            T GetFieldValue(const std::string& p_Name)
            {
                KTN_PROFILE_FUNCTION();

                const auto& fields = m_ScriptClass->GetFields();
                auto it = fields.find(p_Name);
                if (it == fields.end())
                {
                    KTN_CORE_ERROR("Field '{}' not found in script class '{}'", p_Name, m_ScriptClass->GetName());
                    return T();
                }

                ScriptFieldValue value = std::monostate{};
                bool success = ScriptFieldUtils::GetFieldValue(GetManagedObject(), it->second, value);
                if (!success)
                {
                    KTN_CORE_ERROR("Failed to get field '{}' in script class '{}'", p_Name, m_ScriptClass->GetName());
                    return T();
                }

                if (!std::holds_alternative<T>(value))
                {
                    KTN_CORE_ERROR("Field '{}' in script class '{}' is not of type '{}'", p_Name, m_ScriptClass->GetName(), typeid(T).name());
                    return T();
                }
                return std::get<T>(value);
            }

            template<typename T>
            bool SetFieldValue(const std::string& p_Name, T p_Value)
            {
                KTN_PROFILE_FUNCTION();

                const auto& fields = m_ScriptClass->GetFields();
                auto it = fields.find(p_Name);
                if (it == fields.end())
                {
                    KTN_CORE_ERROR("Field '{}' not found in script class '{}'", p_Name, m_ScriptClass->GetName());
                    return false;
                }

                bool success = ScriptFieldUtils::SetFieldValue(GetManagedObject(), it->second, p_Value);
                if (!success)
                {
                    KTN_CORE_ERROR("Failed to set field '{}' in script class '{}'", p_Name, m_ScriptClass->GetFullName());
                    return false;
                }

                return true;
            }

            MonoObject* GetManagedObject();

        private:
            Ref<ScriptClass> m_ScriptClass = nullptr;

            uint32_t m_Handle              = 0;

            MonoMethod* m_Constructor      = nullptr;
            MonoMethod* m_OnCreateMethod   = nullptr;
            MonoMethod* m_OnUpdateMethod   = nullptr;

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
            static ScriptFieldMap& GetScriptFieldMap(const std::string& p_FullClassName);
            static const std::unordered_map<std::string, Ref<ScriptClass>>& GetEntityClasses();
            static const std::unordered_map<UUID, Ref<ScriptInstance>>& GetEntityInstances();
            static MonoImage* GetCoreAssemblyImage();
            static MonoDomain* GetAppDomain();

            static MonoObject* GetManagedInstance(UUID p_UUID);
    };

} // namespace KTN