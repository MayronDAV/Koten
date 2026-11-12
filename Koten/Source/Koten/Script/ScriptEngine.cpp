#include "ktnpch.h"
#include "ScriptEngine.h"
#include "Koten/Core/FileSystem.h"
#include "ScriptGlue.h"
#include "Koten/Core/Application.h"
#include "Koten/Project/Project.h"
#include "Koten/Scene/SceneManager.h"

// lib
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include <mono/metadata/tabledefs.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/exception.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/class.h>
#include <filewatch/FileWatch.hpp>




namespace KTN
{
    namespace
    {
        static std::unordered_map<std::string, ScriptFieldType> s_ScriptFieldTypeMap =
        {
            { "System.Single",  ScriptFieldType::Float   },
            { "System.Double",  ScriptFieldType::Double  },
            { "System.Boolean", ScriptFieldType::Bool    },
            { "System.Char",    ScriptFieldType::Char    },
            { "System.Int16",   ScriptFieldType::Short   },
            { "System.Int32",   ScriptFieldType::Int     },
            { "System.Int64",   ScriptFieldType::Long    },
            { "System.Byte",    ScriptFieldType::Byte    },
            { "System.UInt16",  ScriptFieldType::UShort  },
            { "System.UInt32",  ScriptFieldType::UInt    },
            { "System.UInt64",  ScriptFieldType::ULong   },

            { "System.String",  ScriptFieldType::String  },

            { "KTN.Vector2",    ScriptFieldType::Vector2 },
            { "KTN.Vector3",    ScriptFieldType::Vector3 },
            { "KTN.Vector4",    ScriptFieldType::Vector4 },

            { "KTN.Scene",      ScriptFieldType::Scene   },
            { "KTN.IO.Path",    ScriptFieldType::Path    },

            { "KTN.Object",     ScriptFieldType::Object  }
        };

        struct ScriptEngineData
        {
            MonoDomain* RootDomain       = nullptr;
            MonoDomain* AppDomain        = nullptr;

            MonoAssembly* CoreAssembly   = nullptr;
            MonoImage* CoreAssemblyImage = nullptr;

            MonoAssembly* AppAssembly    = nullptr;
            MonoImage* AppAssemblyImage  = nullptr;
            std::string AppAssemblyPath  = "";

            std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
            std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;
            std::unordered_map<std::string, ScriptFieldMap> ScriptFields;

            Unique<filewatch::FileWatch<std::string>> AppAssemblyFileWatcher;
            bool AssemblyReloadPending = false;
            bool IsRunning = false;

            ScriptClass EntityClass;
            ScriptClass ObjectClass;
        };

        static ScriptEngineData* s_Data = nullptr;

        static void InitMono()
        {
            KTN_PROFILE_FUNCTION();

            mono_set_assemblies_path("Mono/lib");

        #ifndef KTN_DIST
            mono_set_signal_chaining(true);
            mono_set_crash_chaining(true);

            mono_debug_init(MONO_DEBUG_FORMAT_MONO);

            const char* options[] = {
                "--soft-breakpoints",
            };

            mono_jit_parse_options(1, (char**)options);
        #endif

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
            KTN_PROFILE_FUNCTION();

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

        static ScriptFieldType MonoTypeToScriptFieldType(MonoType* p_MonoType)
        {
            KTN_PROFILE_FUNCTION();

            std::string typeName = mono_type_get_name(p_MonoType);

            auto it = s_ScriptFieldTypeMap.find(typeName);
            if (it == s_ScriptFieldTypeMap.end())
            {
                KTN_CORE_ERROR("Unknown type: {}", typeName);
                return ScriptFieldType::None;
            }

            return it->second;
        }

        static MonoClassField* FindFieldInHierarchy(MonoClass* p_Class, const char* p_Name)
        {
            KTN_PROFILE_FUNCTION_LOW();

            for (MonoClass* current = p_Class; current != nullptr; current = mono_class_get_parent(current))
            {
                MonoClassField* field = mono_class_get_field_from_name(current, p_Name);
                if (field)
                    return field;
            }

            return nullptr;
        }

        static MonoProperty* FindPropertyInHierarchy(MonoClass* p_Class, const char* p_Name)
        {
            KTN_PROFILE_FUNCTION_LOW();

            for (MonoClass* current = p_Class; current != nullptr; current = mono_class_get_parent(current))
            {
                MonoProperty* property = mono_class_get_property_from_name(current, p_Name);
                if (property)
                    return property;
            }

            return nullptr;
        }

        static bool MonoStringToNative(MonoString* p_String, std::string& p_OutString)
        {
            KTN_PROFILE_FUNCTION_LOW();

            if (!p_String)
            {
                p_OutString.clear();
                return true;
            }

            char* utf8 = mono_string_to_utf8(p_String);

            if (!utf8)
                return false;

            p_OutString = utf8;
            mono_free(utf8);

            return true;
        }

        static MonoObject* InvokeConstructor(MonoObject* p_Instance, MonoMethod* p_Constructor, void** p_Params)
        {
            KTN_PROFILE_FUNCTION_LOW();

            if (!p_Instance || !p_Constructor)
                return nullptr;

            MonoObject* exception = nullptr;
            mono_runtime_invoke(p_Constructor, p_Instance, p_Params, &exception);

            if (exception)
            {
                KTN_CORE_ERROR("Managed constructor threw an exception.");
                return nullptr;
            }

            return p_Instance;
        }

        static bool SetManagedReference(MonoObject* p_Instance, MonoClassField* p_Field, MonoObject* p_Value)
        {
            KTN_PROFILE_FUNCTION_LOW();

            if (!p_Instance || !p_Field)
                return false;

            mono_field_set_value(p_Instance, p_Field, p_Value);
            return true;
        }

        static MonoObject* GetManagedReference(MonoObject* p_Instance, MonoClassField* p_Field)
        {
            KTN_PROFILE_FUNCTION_LOW();

            if (!p_Instance || !p_Field)
                return nullptr;

            MonoObject* value = nullptr;

            mono_field_get_value(p_Instance, p_Field, &value);
            return value;
        }

        template<typename T>
        static T MonoFieldGetValue(MonoObject* p_Instance, MonoClassField* p_Field)
        {
            T value{};
            mono_field_get_value(p_Instance, p_Field, &value);
            return value;
        }

        static MonoMethod* FindConstructor(MonoClass* p_Class, MonoClass* p_ParameterClass)
        {
            KTN_PROFILE_FUNCTION_LOW();

            void* iterator = nullptr;

            while (MonoMethod* method = mono_class_get_methods(p_Class, &iterator))
            {
                auto name = mono_method_get_name(method);
                if (std::strcmp(name, ".ctor") != 0)
                    continue;

                MonoMethodSignature* signature = mono_method_signature(method);
                uint32_t count = mono_signature_get_param_count(signature);
                if (count != 1)
                    continue;

                void* it = nullptr;
                MonoType* parameterType   = mono_signature_get_params(signature, &it);
                if (!parameterType)
                    return nullptr;

                MonoClass* parameterClass = mono_class_from_mono_type(parameterType);
                if (parameterClass == p_ParameterClass)
                    return method;
            }

            return nullptr;
        }

        template<typename T>
        static bool GetProperty(MonoObject* p_Object, const char* p_Name, T& p_Value)
        {
            KTN_PROFILE_FUNCTION_LOW();

            if (!p_Object)
                return false;

            MonoProperty* property = FindPropertyInHierarchy(mono_object_get_class(p_Object), p_Name);
            if (!property)
                return false;

            MonoMethod* getter = mono_property_get_get_method(property);
            if (!getter)
                return false;

            MonoObject* exception = nullptr;

            MonoObject* result = mono_runtime_invoke(getter, p_Object, nullptr, &exception);
            if (exception || !result)
                return false;

            p_Value = *static_cast<T*>(mono_object_unbox(result));
            return true;
        }

        template<typename T>
        static bool SetProperty(MonoObject* p_Object, const char* p_Name, T p_Value)
        {
            KTN_PROFILE_FUNCTION_LOW();

            if (!p_Object)
                return false;

            MonoProperty* property = FindPropertyInHierarchy(mono_object_get_class(p_Object), p_Name);
            if (!property)
                return false;

            MonoMethod* setter     = mono_property_get_set_method(property);
            if (!setter)
                return false;

            void* args[]           = { &p_Value };
            MonoObject* exception  = nullptr;

            mono_runtime_invoke(setter, p_Object, args, &exception);
            return exception == nullptr;
        }

        template<typename T>
        static T GetFieldOrPropertyValue(MonoClass* p_Class, MonoObject* p_Instance, const std::string& p_Name)
        {
            KTN_PROFILE_FUNCTION_LOW();

            T value{};
            MonoClassField* field = FindFieldInHierarchy(p_Class, p_Name.c_str());
            if (field)
                value = MonoFieldGetValue<T>(p_Instance, field);
            else
            {
                if (!GetProperty<T>(p_Instance, p_Name.c_str(), value))
                {
                    KTN_CORE_ERROR("Failed to get property '{}'", p_Name);
                }
            }

            return value;
        }

        static bool HasAttribute(MonoClassField* p_Field, MonoClass* p_Class, MonoImage* p_Image, const char* p_AttributeName, const char* p_NamespaceName = "KTN") 
        {
            MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_field(p_Class, p_Field);
            if (!attrInfo) return false;

            MonoClass* attributeClass = mono_class_from_name(p_Image, p_NamespaceName, p_AttributeName);
            bool hasAttr = (attributeClass && mono_custom_attrs_has_attr(attrInfo, attributeClass));

            mono_custom_attrs_free(attrInfo);
            return hasAttr;
        }

        static ScriptFieldCategory GetFieldCategory(MonoType* p_Type)
        {
            KTN_PROFILE_FUNCTION_LOW();

            if (!p_Type)
                return ScriptFieldCategory::Unsupported;

            int typeCode = mono_type_get_type(p_Type);
            switch (typeCode)
            {
                case MONO_TYPE_BOOLEAN:
                case MONO_TYPE_CHAR:
                case MONO_TYPE_I1:
                case MONO_TYPE_U1:
                case MONO_TYPE_I2:
                case MONO_TYPE_U2:
                case MONO_TYPE_I4:
                case MONO_TYPE_U4:
                case MONO_TYPE_I8:
                case MONO_TYPE_U8:
                case MONO_TYPE_R4:
                case MONO_TYPE_R8:
                    return ScriptFieldCategory::Primitive;

                case MONO_TYPE_SZARRAY:
                case MONO_TYPE_ARRAY:
                    return ScriptFieldCategory::Array;

                case MONO_TYPE_CLASS:
                case MONO_TYPE_VALUETYPE:
                {
                    MonoClass* fieldClass = mono_class_from_mono_type(p_Type);
                    if (!fieldClass)
                        return ScriptFieldCategory::Unsupported;

                    if (mono_class_is_enum(fieldClass))
                        return ScriptFieldCategory::Enum;

                    if (mono_class_is_valuetype(fieldClass))
                        return ScriptFieldCategory::Struct;

                    return ScriptFieldCategory::Class;
                }

                default:
                    return ScriptFieldCategory::Unsupported;
            }
        }

        static ScriptFieldValue GetScriptFieldValueByType(const ScriptFieldType& p_Type)
        {
            KTN_PROFILE_FUNCTION_LOW();

            #define SCRIPT_FIELD_VALUE(FieldType, Type) \
                case ScriptFieldType::FieldType:        \
                {                                       \
                    value  = Type{};                    \
                    break;                              \
                }                                       \

            ScriptFieldValue value = std::monostate{};
            switch (p_Type)
            {
                SCRIPT_FIELD_VALUE(Float, float);
                SCRIPT_FIELD_VALUE(Double, double);
                SCRIPT_FIELD_VALUE(Bool, bool);
                SCRIPT_FIELD_VALUE(Char, char16_t);
                SCRIPT_FIELD_VALUE(Byte, int8_t);
                SCRIPT_FIELD_VALUE(Short, int16_t);
                SCRIPT_FIELD_VALUE(Int, int32_t);
                SCRIPT_FIELD_VALUE(Long, int64_t);
                SCRIPT_FIELD_VALUE(UByte, uint8_t);
                SCRIPT_FIELD_VALUE(UShort, uint16_t);
                SCRIPT_FIELD_VALUE(UInt, uint32_t);
                SCRIPT_FIELD_VALUE(ULong, uint64_t);
                SCRIPT_FIELD_VALUE(Vector2, glm::vec2);
                SCRIPT_FIELD_VALUE(Vector3, glm::vec3);
                SCRIPT_FIELD_VALUE(Vector4, glm::vec4);
                SCRIPT_FIELD_VALUE(String, std::string);
                SCRIPT_FIELD_VALUE(Path, std::string);
                SCRIPT_FIELD_VALUE(Scene, uint64_t);
                SCRIPT_FIELD_VALUE(Object, ObjectHandle);
            }

            #undef RET_SCRIPT_FIELD
            return value;
        }

        static void LoadAssemblyClasses()
        {
            KTN_PROFILE_FUNCTION();

            s_Data->EntityClasses.clear();

            const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_Data->AppAssemblyImage, MONO_TABLE_TYPEDEF);
            int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
            MonoClass* entityClass = mono_class_from_name(s_Data->CoreAssemblyImage, "KTN", "ScriptBehavior");
            MonoClass* objectClass = mono_class_from_name(s_Data->CoreAssemblyImage, "KTN", "Object");

            for (int32_t i = 0; i < numTypes; i++)
            {
                uint32_t cols[MONO_TYPEDEF_SIZE];
                mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

                const char* nameSpace = mono_metadata_string_heap(s_Data->AppAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
                const char* className = mono_metadata_string_heap(s_Data->AppAssemblyImage, cols[MONO_TYPEDEF_NAME]);
                std::string fullName;
                if (strlen(nameSpace) != 0)
                    fullName = fmt::format("{}.{}", nameSpace, className);
                else
                    fullName = className;

                MonoClass* monoClass = mono_class_from_name(s_Data->AppAssemblyImage, nameSpace, className);

                if (monoClass == entityClass)
                    continue;

                bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false);
                if (!isEntity)
                    continue;

                Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(nameSpace, className);
                s_Data->EntityClasses[fullName] = scriptClass;
                auto& fields = scriptClass->GetFields();

                auto& scriptFieldsMap = s_Data->ScriptFields[fullName];

                int fieldCount = mono_class_num_fields(monoClass);
                //KTN_CORE_WARN("'{}' has {} fields:", fullName, fieldCount);
                void* iterator = nullptr;
                while (MonoClassField* field  = mono_class_get_fields(monoClass, &iterator))
                {
                    const char* fieldName     = mono_field_get_name(field);
                    uint32_t flags            = mono_field_get_flags(field);
                    MonoType* type            = mono_field_get_type(field);
                    if (!type)
                        continue;

                    auto category             = GetFieldCategory(type);
                    if (category == ScriptFieldCategory::Array || category == ScriptFieldCategory::Unsupported)
                        continue;

                    MonoClass* fieldClass     = mono_class_from_mono_type(type);
                    bool isObject             = fieldClass == objectClass || mono_class_is_subclass_of(fieldClass, objectClass, false);

                    ScriptFieldType fieldType = isObject ? ScriptFieldType::Object : MonoTypeToScriptFieldType(type);


                    if (flags & FIELD_ATTRIBUTE_PUBLIC)
                    {
                        //KTN_CORE_WARN("  '{}' ({})", fieldName, Utils::ScriptFieldTypeToString(fieldType));

                        fields[fieldName]          = { fieldType, category, fieldName, false, true, true, field, type, fieldClass };
                        scriptFieldsMap[fieldName] = { fields[fieldName], GetScriptFieldValueByType(fieldType), false };
                    }

                    if (flags & FIELD_ATTRIBUTE_PRIVATE)
                    {
                        bool showInEditor     = HasAttribute(field, monoClass, s_Data->CoreAssemblyImage, "ShowInEditor", "KTN");
                        bool serialize        = HasAttribute(field, monoClass, s_Data->CoreAssemblyImage, "SerializeField", "KTN");
                        if (!showInEditor && !serialize)
                            continue; // Skip private fields that are not marked with ShowInEditor or SerializeField

                        //KTN_CORE_WARN("  '{}' ({}) [Private]", fieldName, Utils::ScriptFieldTypeToString(fieldType));

                        fields[fieldName]          = { fieldType, category, fieldName, true, showInEditor, serialize, field, type, fieldClass };
                        scriptFieldsMap[fieldName] = { fields[fieldName], GetScriptFieldValueByType(fieldType), false };
                    }
                }

                // KTN_CORE_WARN("'{}' has {} fields:", fullName, scriptFieldsMap.size());
            }
        }

        static MonoObject* InstantiateClass(MonoClass* p_Class)
        {
            KTN_PROFILE_FUNCTION();

            MonoObject* instance = mono_object_new(s_Data->AppDomain, p_Class);
            mono_runtime_object_init(instance);
            return instance;
        }

        static void OnAppAssemblyFileSystemEvent(const std::string& p_Path, const filewatch::Event p_ChangeType)
        {
            if (!Engine::Get().GetSettings().AutoRecompile)
                return;

            if (!s_Data->AssemblyReloadPending && (p_ChangeType == filewatch::Event::modified || p_ChangeType == filewatch::Event::added || p_ChangeType == filewatch::Event::removed))
            {
                s_Data->AssemblyReloadPending = true;

                Application::Get().SubmitToMainThread([&]()
                {
                    s_Data->AppAssemblyFileWatcher.reset();
                    ScriptEngine::RecompileAppAssembly();

                    if (s_Data->IsRunning)
                    {
                        s_Data->EntityInstances.clear();
                        for (auto& scene : SceneManager::GetActiveScenes())
                            ScriptEngine::OnRuntimeStart(scene.get());
                    }
                });
            }
        }

        void PrintAssemblyTypes(MonoAssembly* assembly)
        {
            MonoImage* image = mono_assembly_get_image(assembly);
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

        MonoAssembly* CompileMonoScripts(const std::string& p_FolderPath, const std::string& p_OutputDllPath, const std::string& p_EngineCoreDllPath, const std::string& p_MonoAssemblyPath = "Mono/lib")
        {
            KTN_PROFILE_FUNCTION();

            auto coreDll = std::filesystem::absolute(p_EngineCoreDllPath).string();
            auto monoAssemblyPath = std::filesystem::absolute(p_MonoAssemblyPath).string();
            #if defined(KTN_WINDOWS)
                std::string comp = ".\\Mono\\mcs";
            #elif defined(KTN_LINUX)
                std::string comp = "mcs";
            #endif

            #ifndef KTN_DIST
                comp += " -debug";
            #endif

            std::string compileCommand = comp + " -w:0 -target:library -sdk:4.5 -out:" + p_OutputDllPath + 
                " " + p_FolderPath +
            #if defined(KTN_WINDOWS)
                "\\*.cs" +
            #elif defined(KTN_LINUX)
                "/*.cs" +
            #endif
                " -reference:mscorlib.dll,System.dll,System.Core.dll," +
                coreDll + 
                " -lib:" + monoAssemblyPath + "," + coreDll;

            int result = system(compileCommand.c_str());
            if (result != 0)
            {
                KTN_CORE_ERROR("Failed to compile scripts with command: {}", compileCommand);
                return nullptr;
            }

            return LoadMonoAssembly(p_OutputDllPath);
        }
    
        class ObjectClassUtils
        {
            public:
                static bool IsObjectType(MonoClass* p_Class)
                {
                    KTN_PROFILE_FUNCTION_LOW();

                    MonoClass* objClass = s_Data->ObjectClass.GetMonoClass();
                    return p_Class == objClass || mono_class_is_subclass_of(p_Class, objClass, false);
                }

                static MonoObject* Create(MonoClass* p_TargetClass, const ObjectHandle& p_Handle)
                {
                    KTN_PROFILE_FUNCTION_LOW();

                    auto* objClass              = s_Data->ObjectClass.GetMonoClass();
                    MonoObject* baseObject      = mono_object_new(s_Data->AppDomain, objClass);
                    MonoMethod* baseConstructor = s_Data->ObjectClass.GetMethod(".ctor", 3);

                    uint64_t id          = p_Handle.ID;
                    uint64_t sceneHandle = p_Handle.SceneHandle;
                    int32_t type         = p_Handle.Type;

                    void* baseArgs[]      = { &id, &sceneHandle, &type };
                    MonoObject* exception = nullptr;

                    mono_runtime_invoke(baseConstructor, baseObject, baseArgs, &exception);
                    if (exception)
                    {
                        KTN_CORE_ERROR("Faield to create managed object!");
                        return nullptr;
                    }

                    MonoMethod* constructor = FindConstructor(p_TargetClass, objClass);
                    if (!constructor)
                        return nullptr;

                    if ((mono_class_get_flags(p_TargetClass) & TYPE_ATTRIBUTE_ABSTRACT) != 0)
                        return nullptr;

                    MonoObject* instance = mono_object_new(s_Data->AppDomain, p_TargetClass);

                    void* args[] = { baseObject };

                    exception = nullptr;
                    mono_runtime_invoke(constructor, instance, args, &exception);
                    if (exception)
                    {
                        KTN_CORE_ERROR("Faield to create managed object!");
                        return nullptr;
                    }

                    return instance;
                }

                static bool GetHandle(MonoObject* p_Object, ObjectHandle& p_Handle)
                {
                    KTN_PROFILE_FUNCTION_LOW();

                    MonoClass* actualClass = mono_object_get_class(p_Object);
                    if (!IsObjectType(actualClass))
                    {
                        KTN_CORE_ERROR("Managed object does not derive from KTN.Object.");
                        return false;
                    }

                    ObjectHandle handle = {};
                    handle.ID           = GetFieldOrPropertyValue<uint64_t>(actualClass, p_Object, "ID");
                    handle.SceneHandle  = GetFieldOrPropertyValue<uint64_t>(actualClass, p_Object, "SceneHandle");
                    handle.Type         = GetFieldOrPropertyValue<int32_t>(actualClass, p_Object, "Type");
                    p_Handle            = handle;
                    return true;
                }
        };

    } // namespace

    #pragma region ScriptEngine

    void ScriptEngine::Init()
    {
        KTN_PROFILE_FUNCTION();

        s_Data = new ScriptEngineData();

        InitMono();
        ScriptGlue::RegisterFunctions();

        bool status = LoadAssembly("Resources/Scripts/Koten-ScriptCore.dll");
        if (!status)
        {
            KTN_CORE_ERROR("[ScriptEngine] Could not load Koten-ScriptCore assembly.");
            return;
        }

        s_Data->EntityClass = ScriptClass("KTN", "ScriptBehavior", true);
        s_Data->ObjectClass = ScriptClass("KTN", "Object", true);
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

    bool ScriptEngine::LoadAppAssembly(const std::string& p_Path)
    {
        KTN_PROFILE_FUNCTION();

        s_Data->AppAssembly = LoadMonoAssembly(p_Path);
        if (!s_Data->AppAssembly)
        {
            KTN_CORE_ERROR("Failed to load assembly: {}", p_Path);
            return false;
        }

        s_Data->AppAssemblyImage = mono_assembly_get_image(s_Data->AppAssembly);

        std::string sourcePath = (Project::GetAssetDirectory() / "Scripts").string();
        if (FileSystem::Exists(sourcePath) && Engine::Get().GetSettings().AutoRecompile)
        {
            s_Data->AppAssemblyFileWatcher = CreateUnique<filewatch::FileWatch<std::string>>(sourcePath, OnAppAssemblyFileSystemEvent);
            s_Data->AssemblyReloadPending = false;
        }

        LoadAssemblyClasses();
        ScriptGlue::RegisterComponents();
        s_Data->AppAssemblyPath = p_Path;
        return true;
    }

    bool ScriptEngine::CompileLoadAppAssembly(bool p_ForceCompile)
    {
        KTN_PROFILE_FUNCTION();

        auto outputDllPath = Project::GetAssetDirectory() / "Scripts.dll";
        auto sourcePath = (Project::GetAssetDirectory() / "Scripts").string();

        if (!p_ForceCompile && FileSystem::Exists(outputDllPath.string()))
            return LoadAppAssembly(outputDllPath.string());

        if (!FileSystem::Exists(sourcePath))
            return false;

        bool existsSourceFiles = false;
        for (const auto& entry : std::filesystem::directory_iterator(sourcePath))
        {
            if (entry.path().extension() == ".cs")
                existsSourceFiles = true;
        }
        if (!existsSourceFiles)
            return false;

        std::filesystem::create_directories(outputDllPath.parent_path());
        s_Data->AppAssembly = CompileMonoScripts(sourcePath, outputDllPath.string(), "Resources/Scripts/Koten-ScriptCore.dll");
        if (!s_Data->AppAssembly)
        {
            KTN_CORE_ERROR("[ScriptEngine] Failed to compile scripts in path: {}", sourcePath);
            return false;
        }

        s_Data->AppAssemblyImage = mono_assembly_get_image(s_Data->AppAssembly);

        if (Engine::Get().GetSettings().AutoRecompile)
        {
            s_Data->AppAssemblyFileWatcher = CreateUnique<filewatch::FileWatch<std::string>>(sourcePath, OnAppAssemblyFileSystemEvent);
            s_Data->AssemblyReloadPending = false;
        }

        LoadAssemblyClasses();
        ScriptGlue::RegisterComponents();
        s_Data->AppAssemblyPath = outputDllPath.string();
        KTN_CORE_INFO("[ScriptEngine] C# Script compiled successfully!");
        return true;
    }

    bool ScriptEngine::CompileScripts(const std::filesystem::path& p_SourcePath, const std::filesystem::path& p_OutFolder)
    {
        KTN_PROFILE_FUNCTION();

        auto outputDllPath = p_OutFolder / "Scripts.dll";
        auto sourcePath = p_SourcePath.string();

        if (!FileSystem::Exists(sourcePath))
            return false;

        bool existsSourceFiles = false;
        for (const auto& entry : std::filesystem::directory_iterator(sourcePath))
        {
            if (entry.path().extension() == ".cs")
            {
                existsSourceFiles = true;
                break;
            }
        }
        if (!existsSourceFiles)
            return false;

        std::filesystem::create_directories(outputDllPath.parent_path());
        CompileMonoScripts(sourcePath, outputDllPath.string(), "Resources/Scripts/Koten-ScriptCore.dll");
        return true;
    }

    void ScriptEngine::OnRuntimeStart(Scene* p_Scene)
    {
        KTN_PROFILE_FUNCTION();

        s_Data->IsRunning = true;

        p_Scene->GetRegistry().view<RuntimeComponent, ScriptComponent>().each(
        [&](auto p_Entt, RuntimeComponent& p_Runtime, ScriptComponent& p_Sc)
        {
            if (!p_Runtime.Enabled) return;

            auto entity = Entity(p_Entt, p_Scene);
            OnCreateEntity(entity);
        });
    }

    void ScriptEngine::OnRuntimeUpdate(Scene* p_Scene)
    {
        KTN_PROFILE_FUNCTION();

        p_Scene->GetRegistry().view<RuntimeComponent, ScriptComponent>().each(
        [&](auto p_Entt, RuntimeComponent& p_Runtime, ScriptComponent& p_Sc)
        {
            if (!p_Runtime.Enabled || !p_Runtime.Active) return;

            auto entity = Entity(p_Entt, p_Scene);
            OnUpdateEntity(entity);
        });
    }

    void ScriptEngine::OnRuntimeStop()
    {
        KTN_PROFILE_FUNCTION();

        s_Data->IsRunning = false;
        s_Data->EntityInstances.clear();
    }

    void ScriptEngine::ReloadAssembly()
    {
        KTN_PROFILE_FUNCTION();

        if (s_Data->AppAssemblyPath.empty())
        {
            KTN_CORE_ERROR("[ScriptEngine] App assembly path is empty. Cannot reload assembly.");
            return;
        }

        mono_domain_set(mono_get_root_domain(), false);
        mono_domain_unload(s_Data->AppDomain);
        s_Data->AppDomain = nullptr;

        bool status = LoadAssembly("Resources/Scripts/Koten-ScriptCore.dll");
        if (!status)
        {
            KTN_CORE_ERROR("[ScriptEngine] Could not load Koten-ScriptCore assembly.");
            return;
        }

        status = LoadAppAssembly(s_Data->AppAssemblyPath);
        if (!status)
        {
            KTN_CORE_ERROR("[ScriptEngine] Could not load app assembly: {0}", s_Data->AppAssemblyPath);
            return;
        }

        // Retrieve and instantiate class
        s_Data->EntityClass = ScriptClass("KTN", "ScriptBehavior", true);
        s_Data->ObjectClass = ScriptClass("KTN", "Object", true);
    }

    void ScriptEngine::RecompileAppAssembly()
    {
        KTN_PROFILE_FUNCTION();

        mono_domain_set(mono_get_root_domain(), false);
        mono_domain_unload(s_Data->AppDomain);
        s_Data->AppDomain = nullptr;

        bool status = LoadAssembly("Resources/Scripts/Koten-ScriptCore.dll");
        if (!status)
        {
            KTN_CORE_ERROR("[ScriptEngine] Could not load Koten-ScriptCore assembly.");
            return;
        }

        status = CompileLoadAppAssembly(true);
        if (!status)
        {
            KTN_CORE_ERROR("[ScriptEngine] Could not compile scripts!");
            return;
        }

        // Retrieve and instantiate class
        s_Data->EntityClass = ScriptClass("KTN", "ScriptBehavior", true);
        s_Data->ObjectClass = ScriptClass("KTN", "Object", true);
    }

    bool ScriptEngine::EntityClassExists(const std::string& p_FullClassName)
    {
        return s_Data->EntityClasses.find(p_FullClassName) != s_Data->EntityClasses.end();
    }

    void ScriptEngine::OnCreateEntity(Entity p_Entity)
    {
        KTN_PROFILE_FUNCTION();

        auto sc = p_Entity.TryGetComponent<ScriptComponent>();
        if (!sc) return;

        auto& rc = p_Entity.GetComponent<RuntimeComponent>();
        if (!rc.Active) return;

        if (ScriptEngine::EntityClassExists(sc->FullClassName))
        {
            UUID entityID = p_Entity.GetUUID();

            Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_Data->EntityClasses[sc->FullClassName], p_Entity);
            s_Data->EntityInstances[entityID] = instance;

            // Copy field values
            auto it = s_Data->ScriptFields.find(sc->FullClassName);
            if (it != s_Data->ScriptFields.end())
            {
                for (const auto& [name, fieldInstance] : it->second)
                {
                    if (!fieldInstance.Field.IsPrivate && fieldInstance.IsChanged)
                        ScriptFieldUtils::SetFieldValue(instance->GetManagedObject(), fieldInstance.Field, fieldInstance.Value);
                }
            }

            instance->InvokeOnCreate();
        }
    }

    void ScriptEngine::OnUpdateEntity(Entity p_Entity)
    {
        KTN_PROFILE_FUNCTION();

        auto& rc = p_Entity.GetComponent<RuntimeComponent>();
        if (!rc.Enabled || !rc.Active) return;

        UUID entityUUID = p_Entity.GetUUID();
        if (s_Data->EntityInstances.find(entityUUID) == s_Data->EntityInstances.end())
        {
            KTN_CORE_ERROR("Script instance not found for entity: {}", (uint64_t)entityUUID);
            return;
        }

        Ref<ScriptInstance> instance = s_Data->EntityInstances[entityUUID];
        instance->InvokeOnUpdate();
    }

    MonoString* ScriptEngine::CreateString(const char* p_String)
    {
        return mono_string_new(s_Data->AppDomain, p_String);
    }

    Ref<ScriptClass> ScriptEngine::GetEntityClass(const std::string& p_Name)
    {
        KTN_PROFILE_FUNCTION();

        if (s_Data->EntityClasses.find(p_Name) == s_Data->EntityClasses.end())
            return nullptr;

        return s_Data->EntityClasses.at(p_Name);
    }

    Ref<ScriptInstance> ScriptEngine::GetEntityScriptInstance(UUID p_EntityID)
    {
        KTN_PROFILE_FUNCTION();

        auto it = s_Data->EntityInstances.find(p_EntityID);
        if (it != s_Data->EntityInstances.end())
            return it->second;

        return nullptr;
    }

    ScriptFieldMap& ScriptEngine::GetScriptFieldMap(const std::string& p_FullClassName)
    {
        KTN_PROFILE_FUNCTION();
        KTN_CORE_ASSERT(!p_FullClassName.empty());

        return s_Data->ScriptFields[p_FullClassName];
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

    MonoDomain* ScriptEngine::GetAppDomain()
    {
        return s_Data->AppDomain;
    }

    MonoObject* ScriptEngine::GetManagedInstance(UUID p_UUID)
    {
        if (s_Data->EntityInstances.find(p_UUID) == s_Data->EntityInstances.end())
        {
            KTN_CORE_ERROR("Script instance not found for entity: {}", (uint64_t)p_UUID);
            return nullptr;
        }

        return s_Data->EntityInstances.at(p_UUID)->GetManagedObject();
    }

    #pragma endregion

    #pragma region ScriptClass

    ScriptClass::ScriptClass(const std::string& p_Namespace, const std::string& p_Name, bool p_IsCore)
        : m_Name(p_Name), m_Namespace(p_Namespace), m_Class(nullptr)
    {
        KTN_PROFILE_FUNCTION();

        m_Class = mono_class_from_name(p_IsCore ? s_Data->CoreAssemblyImage : s_Data->AppAssemblyImage, p_Namespace.c_str(), p_Name.c_str());
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
        KTN_PROFILE_FUNCTION();

        MonoObject* exception = nullptr;
        MonoObject* result = mono_runtime_invoke(p_Method, p_Instance, p_Params, &exception);

        if (exception)
            mono_print_unhandled_exception(exception);

        return result;
    }

    #pragma endregion

    #pragma region ScriptInstance

    ScriptInstance::ScriptInstance(Ref<ScriptClass> p_ScriptClass, Entity p_Entity)
        : m_ScriptClass(p_ScriptClass)
    {
        KTN_PROFILE_FUNCTION();

        auto instance    = p_ScriptClass->Instantiate();

        m_Handle         = mono_gchandle_new(instance, false);

        m_Constructor    = s_Data->EntityClass.GetMethod(".ctor", 2);
        m_OnCreateMethod = p_ScriptClass->GetMethod("OnCreate", 0);
        m_OnUpdateMethod = p_ScriptClass->GetMethod("OnUpdate", 0);

        // Call constructor
        {
            UUID entityID           = p_Entity.GetUUID();
            AssetHandle sceneHandle = p_Entity.GetScene()->Handle;
            void* param[]           = { &entityID, &sceneHandle };
            m_ScriptClass->InvokeMethod(instance, m_Constructor, param);
        }
    }

    ScriptInstance::~ScriptInstance()
    {
        if (m_Handle)
        {
            mono_gchandle_free(m_Handle);
            m_Handle = 0;
        }
    }

    void ScriptInstance::InvokeOnCreate()
    {
        KTN_PROFILE_FUNCTION();

        MonoObject* instance = GetManagedObject();
        if (!instance)
            return;

        if (m_OnCreateMethod)
            m_ScriptClass->InvokeMethod(instance, m_OnCreateMethod);
    }

    void ScriptInstance::InvokeOnUpdate()
    {
        KTN_PROFILE_FUNCTION();

        MonoObject* instance = GetManagedObject();
        if (!instance)
            return;

        if (m_OnUpdateMethod)
            m_ScriptClass->InvokeMethod(instance, m_OnUpdateMethod);
    }

    void ScriptInstance::TryInvokeMethod(const std::string& p_MethodName, int p_Count, void** p_Params)
    {
        KTN_PROFILE_FUNCTION();

        MonoObject* instance = GetManagedObject();
        if (!instance)
            return;

        auto method = m_ScriptClass->GetMethod(p_MethodName, p_Count);
        if (method)
            m_ScriptClass->InvokeMethod(instance, method, p_Params);
    }

    void ScriptInstance::TryInvokeParentMethod(const std::string& p_MethodName, int p_Count, void** p_Params)
    {
        KTN_PROFILE_FUNCTION();

        MonoObject* instance = GetManagedObject();
        if (!instance)
            return;

        MonoClass* currentClass = mono_object_get_class(instance);
        MonoClass* parentClass = mono_class_get_parent(currentClass);

        if (parentClass) 
        {
            MonoMethod* method = mono_class_get_method_from_name(parentClass, p_MethodName.c_str(), p_Count);
            if (method)
                m_ScriptClass->InvokeMethod(instance, method, p_Params);
        }
    }

    MonoObject* ScriptInstance::GetManagedObject()
    {
        if (!m_Handle)
            return nullptr;

        return mono_gchandle_get_target(m_Handle);
    }

    #pragma endregion

    bool ScriptFieldUtils::SetFieldValue(MonoObject* p_Instance, const ScriptField& p_Field, const ScriptFieldValue& p_Value)
    {
        KTN_PROFILE_FUNCTION();

        if (!p_Instance || !p_Field.Field || !p_Field.FieldClass)
            return false;

        return std::visit([&](const auto& typedValue) -> bool
        {
            using T = std::decay_t<decltype(typedValue)>;

            if constexpr (std::is_same_v<T, std::monostate>)
            {
                return false;
            }
            else if constexpr (std::is_same_v<T, uint64_t>)
            {
                if (p_Field.Type == ScriptFieldType::Scene)
                {
                    MonoClass* sceneClass     = mono_class_from_name(s_Data->CoreAssemblyImage, "KTN", "Scene");
                    if (!sceneClass || p_Field.FieldClass != sceneClass)
                        return false;

                    MonoMethod* constructor   = mono_class_get_method_from_name(sceneClass, ".ctor", 1);
                    if (!constructor)
                    {
                        KTN_CORE_ERROR("Failed to find KTN.Scene constructor.");
                        return false;
                    }

                    MonoObject* sceneInstance = mono_object_new(s_Data->AppDomain, sceneClass);
                    if (!sceneInstance)
                        return false;

                    uint64_t handle           = typedValue;
                    void* params[]            = { &handle };

                    if (!InvokeConstructor(sceneInstance, constructor, params))
                        return false;

                    return SetManagedReference(p_Instance, p_Field.Field, sceneInstance);
                }

                if (p_Field.Type != ScriptFieldType::ULong || mono_type_get_type(p_Field.FieldType) != MONO_TYPE_U8)
                {
                    return false;
                }

                uint64_t value = typedValue;
                mono_field_set_value(p_Instance, p_Field.Field, &value);
                return true;
            }
            else if constexpr (std::is_same_v<T, std::string>)
            {
                if (p_Field.Type == ScriptFieldType::String)
                {
                    if (p_Field.FieldClass != mono_get_string_class())
                        return false;

                    MonoString* string = ScriptEngine::CreateString(typedValue.c_str());
                    if (!string)
                        return false;

                    return SetManagedReference(p_Instance, p_Field.Field, reinterpret_cast<MonoObject*>(string));
                }

                if (p_Field.Type == ScriptFieldType::Path)
                {
                    MonoClass* pathClass    = mono_class_from_name(s_Data->CoreAssemblyImage, "KTN.IO", "Path");
                    if (!pathClass || p_Field.FieldClass != pathClass)
                        return false;

                    MonoMethod* constructor = mono_class_get_method_from_name(pathClass, ".ctor", 1);
                    if (!constructor)
                    {
                        KTN_CORE_ERROR("Failed to find KTN.IO.Path constructor.");
                        return false;
                    }

                    MonoString* pathString  = ScriptEngine::CreateString(typedValue.c_str());
                    if (!pathString)
                        return false;

                    MonoObject* pathInstance = mono_object_new(s_Data->AppDomain, pathClass);
                    if (!pathInstance)
                        return false;

                    void* params[] = { pathString };
                    if (!InvokeConstructor(pathInstance, constructor, params))
                        return false;

                    return SetManagedReference(p_Instance, p_Field.Field, pathInstance);
                }

                return false;
            }
            else if constexpr (std::is_same_v<T, ObjectHandle>)
            {
                if (p_Field.Type != ScriptFieldType::Object)
                    return false;

                if (!ObjectClassUtils::IsObjectType(p_Field.FieldClass))
                    return false;

                MonoObject* objInstance = ObjectClassUtils::Create(p_Field.FieldClass, typedValue);
                if (!objInstance)
                    return false;

                return SetManagedReference(p_Instance, p_Field.Field, objInstance);
            }
            else
            {
                if (!mono_class_is_valuetype(p_Field.FieldClass))
                    return false;

                if constexpr (std::is_same_v<T, bool>)
                {
                    if (mono_type_get_type(p_Field.FieldType) != MONO_TYPE_BOOLEAN)
                        return false;
                }
                else if constexpr (std::is_same_v<T, char16_t>)
                {
                    if (mono_type_get_type(p_Field.FieldType) != MONO_TYPE_CHAR)
                        return false;
                }
                else if constexpr (std::is_same_v<T, int8_t>)
                {
                    if (mono_type_get_type(p_Field.FieldType) != MONO_TYPE_I1)
                        return false;
                }
                else if constexpr (std::is_same_v<T, uint8_t>)
                {
                    if (mono_type_get_type(p_Field.FieldType) != MONO_TYPE_U1)
                        return false;
                }
                else if constexpr (std::is_same_v<T, int16_t>)
                {
                    if (mono_type_get_type(p_Field.FieldType) != MONO_TYPE_I2)
                        return false;
                }
                else if constexpr (std::is_same_v<T, uint16_t>)
                {
                    if (mono_type_get_type(p_Field.FieldType) != MONO_TYPE_U2)
                        return false;
                }
                else if constexpr (std::is_same_v<T, int32_t>)
                {
                    if (mono_type_get_type(p_Field.FieldType) != MONO_TYPE_I4)
                        return false;
                }
                else if constexpr (std::is_same_v<T, uint32_t>)
                {
                    if (mono_type_get_type(p_Field.FieldType) != MONO_TYPE_U4)
                        return false;
                }
                else if constexpr (std::is_same_v<T, int64_t>)
                {
                    if (mono_type_get_type(p_Field.FieldType) != MONO_TYPE_I8)
                        return false;
                }
                else if constexpr (std::is_same_v<T, uint64_t>)
                {
                    if (mono_type_get_type(p_Field.FieldType) != MONO_TYPE_U8)
                        return false;
                }
                else if constexpr (std::is_same_v<T, float>)
                {
                    if (mono_type_get_type(p_Field.FieldType) != MONO_TYPE_R4)
                        return false;
                }
                else if constexpr (std::is_same_v<T, double>)
                {
                    if (mono_type_get_type(p_Field.FieldType) != MONO_TYPE_R8)
                        return false;
                }
                else if constexpr (std::is_same_v<T, glm::vec2>)
                {
                    MonoClass* expectedClass = mono_class_from_name(s_Data->CoreAssemblyImage, "KTN", "Vector2");
                    if (!expectedClass || p_Field.FieldClass != expectedClass)
                        return false;
                }
                else if constexpr (std::is_same_v<T, glm::vec3>)
                {
                    MonoClass* expectedClass = mono_class_from_name(s_Data->CoreAssemblyImage, "KTN", "Vector3");
                    if (!expectedClass || p_Field.FieldClass != expectedClass)
                        return false;
                }
                else if constexpr (std::is_same_v<T, glm::vec4>)
                {
                    MonoClass* expectedClass = mono_class_from_name(s_Data->CoreAssemblyImage, "KTN", "Vector4");
                    if (!expectedClass || p_Field.FieldClass != expectedClass)
                        return false;
                }

                mono_field_set_value(p_Instance, p_Field.Field, const_cast<T*>(&typedValue));
                return true;
            }
        },
        p_Value);
    }

    bool ScriptFieldUtils::GetFieldValue(MonoObject* p_Instance, const ScriptField& p_Field, ScriptFieldValue& p_OutValue)
    {
        KTN_PROFILE_FUNCTION();

        if (!p_Instance || !p_Field.Field || !p_Field.FieldClass)
            return false;

        switch (p_Field.Type)
        {
            case ScriptFieldType::None:
                p_OutValue = std::monostate{};
                return false;

            case ScriptFieldType::Bool:
            {
                p_OutValue = MonoFieldGetValue<bool>(p_Instance, p_Field.Field);
                return true;
            }

            case ScriptFieldType::Char:
            {
                p_OutValue = MonoFieldGetValue<char16_t>(p_Instance, p_Field.Field);
                return true;
            }

            case ScriptFieldType::Byte:
            {
                p_OutValue = MonoFieldGetValue<int8_t>(p_Instance, p_Field.Field);
                return true;
            }

            case ScriptFieldType::UByte:
            {
                p_OutValue = MonoFieldGetValue<uint8_t>(p_Instance, p_Field.Field);
                return true;
            }

            case ScriptFieldType::Short:
            {
                p_OutValue = MonoFieldGetValue<int16_t>(p_Instance, p_Field.Field);
                return true;
            }

            case ScriptFieldType::UShort:
            {
                p_OutValue = MonoFieldGetValue<uint16_t>(p_Instance, p_Field.Field);
                return true;
            }

            case ScriptFieldType::Int:
            {
                p_OutValue = MonoFieldGetValue<int32_t>(p_Instance, p_Field.Field);
                return true;
            }

            case ScriptFieldType::UInt:
            {
                p_OutValue = MonoFieldGetValue<uint32_t>(p_Instance, p_Field.Field);
                return true;
            }

            case ScriptFieldType::Long:
            {
                p_OutValue = MonoFieldGetValue<int64_t>(p_Instance, p_Field.Field);
                return true;
            }

            case ScriptFieldType::ULong:
            {
                p_OutValue = MonoFieldGetValue<uint64_t>(p_Instance, p_Field.Field);
                return true;
            }

            case ScriptFieldType::Float:
            {
                p_OutValue = MonoFieldGetValue<float>(p_Instance, p_Field.Field);
                return true;
            }

            case ScriptFieldType::Double:
            {
                p_OutValue = MonoFieldGetValue<double>(p_Instance, p_Field.Field);
                return true;
            }

            case ScriptFieldType::Vector2:
            {
                p_OutValue = MonoFieldGetValue<glm::vec2>(p_Instance, p_Field.Field);
                return true;
            }

            case ScriptFieldType::Vector3:
            {
                p_OutValue = MonoFieldGetValue<glm::vec3>(p_Instance, p_Field.Field);
                return true;
            }

            case ScriptFieldType::Vector4:
            {
                p_OutValue = MonoFieldGetValue<glm::vec4>(p_Instance, p_Field.Field);
                return true;
            }

            case ScriptFieldType::String:
            {
                MonoObject* reference = GetManagedReference(p_Instance, p_Field.Field);
                if (!reference)
                {
                    p_OutValue = std::string{};
                    return true;
                }

                MonoString* monoString = reinterpret_cast<MonoString*>(reference);

                std::string value;
                if (!MonoStringToNative(monoString, value))
                    return false;

                p_OutValue = std::move(value);
                return true;
            }

            case ScriptFieldType::Path:
            {
                MonoObject* pathInstance = GetManagedReference(p_Instance, p_Field.Field);
                if (!pathInstance)
                {
                    p_OutValue = std::string{};
                    return true;
                }

                MonoClass* pathClass = mono_class_from_name(s_Data->CoreAssemblyImage, "KTN.IO", "Path");
                if (!pathClass)
                {
                    KTN_CORE_ERROR("Failed to find KTN.IO.Path class.");
                    return false;
                }

                MonoClass* actualClass = mono_object_get_class(pathInstance);

                if (actualClass != pathClass && !mono_class_is_subclass_of(actualClass, pathClass, false))
                {
                    KTN_CORE_ERROR("Invalid managed type for KTN.IO.Path field.");
                    return false;
                }

                MonoString* pathString = nullptr;

                MonoClassField* pathField = FindFieldInHierarchy(actualClass, "m_Path");
                if (pathField)
                    mono_field_get_value(pathInstance, pathField, &pathString);
                else
                {
                    MonoProperty* property = FindPropertyInHierarchy(actualClass, "m_Path");
                    if (!property)
                        return false;

                    MonoMethod* getter = mono_property_get_get_method(property);
                    if (!getter)
                        return false;

                    MonoObject* exception = nullptr;

                    MonoObject* result = mono_runtime_invoke(getter, pathInstance, nullptr, &exception);
                    if (exception || !result)
                        return false;

                    pathString = static_cast<MonoString*>(mono_object_unbox(result));
                    if (!pathString)
                        return false;
                }

                std::string value;
                if (!MonoStringToNative(pathString, value))
                    return false;

                p_OutValue = std::move(value);
                return true;
            }

            case ScriptFieldType::Scene:
            {
                MonoObject* sceneInstance = GetManagedReference(p_Instance, p_Field.Field);
                if (!sceneInstance)
                {
                    p_OutValue = uint64_t{ 0 };
                    return true;
                }

                MonoClass* sceneClass = mono_class_from_name(s_Data->CoreAssemblyImage, "KTN", "Scene");
                if (!sceneClass)
                {
                    KTN_CORE_ERROR("Failed to find KTN.Scene class.");
                    return false;
                }

                MonoClass* actualClass = mono_object_get_class(sceneInstance);
                if (actualClass != sceneClass && !mono_class_is_subclass_of(actualClass, sceneClass, false))
                {
                    KTN_CORE_ERROR("Invalid managed type for KTN.Scene field.");
                    return false;
                }

                p_OutValue = GetFieldOrPropertyValue<uint64_t>(actualClass, sceneInstance, "Handle");
                return true;
            }

            case ScriptFieldType::Object:
            {
                MonoObject* objectInstance = GetManagedReference(p_Instance, p_Field.Field);
                if (!objectInstance)
                {
                    p_OutValue = ObjectHandle{};
                    return true;
                }

                ObjectHandle handle = {};
                if (!ObjectClassUtils::GetHandle(objectInstance, handle))
                {
                    p_OutValue = ObjectHandle{};
                    return false;
                }

                p_OutValue          = handle;
                return true;
            }
        }

        return false;
    }

} // namespace KTN
