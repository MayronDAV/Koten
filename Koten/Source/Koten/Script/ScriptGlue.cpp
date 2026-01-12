#include "ktnpch.h"
#include "ScriptGlue.h"
#include "ScriptEngine.h"
#include "Koten/OS/KeyCodes.h"
#include "Koten/OS/MouseCodes.h"
#include "Koten/OS/Input.h"
#include "Koten/OS/GamepadCodes.h"
#include "Koten/Project/Project.h"
#include "Koten/Scene/SceneManager.h"
#include "Koten/Systems/B2Physics.h"
#include "Koten/Asset/PrefabImporter.h"

// lib
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <box2d/box2d.h>



namespace KTN
{
	namespace
	{
		static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;

		struct ObjectHandle
		{
			uint64_t ID;
			uint64_t SceneHandle;
			int32_t Type;
		};

		static std::string MonoStringToString(MonoString* p_String)
		{
			KTN_PROFILE_FUNCTION_LOW();

			char* cStr = mono_string_to_utf8(p_String);
			std::string str(cStr);
			mono_free(cStr);
			return str;
		}

		static B2BodyID GetPhysicsBody2D(Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION_LOW();

			if (p_Entity.HasComponent<Rigidbody2DComponent>())
				return p_Entity.GetComponent<Rigidbody2DComponent>().Body;
			else if (p_Entity.HasComponent<CharacterBody2DComponent>())
				return p_Entity.GetComponent<CharacterBody2DComponent>().Body;
			else if (p_Entity.HasComponent<StaticBody2DComponent>())
				return p_Entity.GetComponent<StaticBody2DComponent>().Body;

			return {};
		}

		static Entity FindWithUUID(ObjectHandle p_Handle)
		{
			if (!p_Handle.SceneHandle)
				return SceneManager::GetEntityByUUID(p_Handle.ID);

			auto scene = AssetManager::Get()->GetAsset<Scene>(p_Handle.SceneHandle);
			return scene->GetEntityByUUID(p_Handle.ID);
		}

		#define KTN_ADD_INTERNAL_CALL(Name) mono_add_internal_call("KTN.InternalCalls::" #Name, Name)

		static MonoObject* GetScriptInstance(UUID p_UUID)
		{
			return ScriptEngine::GetManagedInstance(p_UUID);
		}
		
		#pragma region Object
		static ObjectHandle Object_Instantiate(ObjectHandle p_Handle, glm::vec3* p_Position, glm::vec3* p_Rotation)
		{
			KTN_PROFILE_FUNCTION_LOW();

			auto type = (AssetType)p_Handle.Type;

			uint64_t sceneHandle = p_Handle.SceneHandle;
			if (!sceneHandle && (type == AssetType::None || type == AssetType::Prefab))
			{
				auto& scenes = SceneManager::GetActiveScenes();
				if (scenes.empty())
				{
					KTN_CORE_ERROR("Failed to instantiate the object. Please provide a valid scene handle! Handle: (ID: {}, SceneHandle: {}, Type: {})",
						p_Handle.ID, sceneHandle, p_Handle.Type);
					return { 0, 0, 0 };
				}

				sceneHandle = scenes[0]->Handle;
			}

			if (type == AssetType::None)
			{
				auto scene = SceneManager::GetScene(sceneHandle);
				auto entity = scene->GetEntityByUUID(p_Handle.ID);
				if (!entity)
				{
					KTN_CORE_ERROR("Failed to instantiate the object. Please provide a valid game object id! Handle: (ID: {}, SceneHandle: {}, Type: {})",
						p_Handle.ID, sceneHandle, p_Handle.Type);
					return { 0, 0, 0 };
				}

				auto newEntity = Scene::DuplicateEntity(entity);
				if (!newEntity)
				{
					KTN_CORE_ERROR("Failed to instantiate the object. Duplicate failed! Handle: (ID: {}, SceneHandle: {}, Type: {})",
						p_Handle.ID, sceneHandle, p_Handle.Type);
					return { 0, 0, 0 };
				}

				auto pos = p_Position ? *p_Position : glm::vec3{0.0f};
				auto rot = p_Rotation ? *p_Rotation : glm::vec3{ 0.0f };
				
				scene->SetEntityTransform(newEntity, pos, rot);

				return { newEntity.GetUUID(), sceneHandle, 0 };
			}

			if (type == AssetType::Prefab)
			{
				KTN_CORE_INFO("Loading Preafab! Handle: (ID: {}, SceneHandle: {}, Type: {})",
					p_Handle.ID, sceneHandle, p_Handle.Type);

				auto& metadata = AssetManager::Get()->GetMetadata(p_Handle.ID);
				delete metadata.AssetData;
				metadata.AssetData = nullptr;
				metadata.AssetData = new PrefabContext{ 0, (AssetHandle)sceneHandle };

				auto prefab = AssetManager::Get()->LoadAsset<Prefab>(p_Handle.ID, metadata);
				if (!prefab)
				{
					KTN_CORE_ERROR("Failed to instantiate the object. Please provide a valid prefab handle! Handle: (ID: {}, SceneHandle: {}, Type: {})", 
						p_Handle.ID, sceneHandle, p_Handle.Type);
					return { 0, 0, 0 };
				}

				auto& entt = prefab->Entt;
				auto scene = entt.GetScene();

				auto pos = p_Position ? *p_Position : glm::vec3{ 0.0f };
				auto rot = p_Rotation ? *p_Rotation : glm::vec3{ 0.0f };

				auto tcomp = entt.TryGetComponent<TransformComponent>();
				if (tcomp)
				{
					tcomp->SetLocalTranslation(pos);
					tcomp->SetLocalRotation(rot);
				}

				if (scene->GetSystemManager()->HasSystem<B2Physics>())
				{
					auto system = scene->GetSystemManager()->GetSystem<B2Physics>();
					if (system->IsRunning())
					{
						system->OnCreateEntity(entt);
					}
				}

				return { entt.GetUUID(), sceneHandle, 0 };
			}

			// Load the asset
			auto& metadata = AssetManager::Get()->GetMetadata(p_Handle.ID);
			auto asset = AssetManager::Get()->LoadAsset<Asset>(p_Handle.ID, metadata);

			return p_Handle;
		}
		#pragma endregion

		#pragma region AssetManager
		static ObjectHandle AssetManager_FindWithPath(MonoString* p_Path)
		{
			KTN_PROFILE_FUNCTION_LOW();

			auto path = MonoStringToString(p_Path);
			path = Project::GetAssetFileSystemPath(path).string();
			auto handle = AssetManager::Get()->GetHandleByPath(path);
			auto type = AssetManager::Get()->GetAssetType(handle);
			return { handle, 0, (int32_t)type };
		}

		static bool AssetManager_IsAssetHandleValid(AssetHandle p_Handle)
		{
			KTN_PROFILE_FUNCTION_LOW();

			return AssetManager::Get()->IsAssetHandleValid(p_Handle);
		}

		static bool AssetManager_IsAssetLoaded(AssetHandle p_Handle)
		{
			KTN_PROFILE_FUNCTION_LOW();

			return AssetManager::Get()->IsAssetLoaded(p_Handle);
		}

		#pragma endregion

		#pragma region Time
		static float Time_GetTime()
		{
			return (float)Time::GetTime();
		}

		static float Time_GetDeltaTime()
		{
			return (float)Time::GetDeltaTime();
		}
		#pragma endregion

		#pragma region GameObject
		static bool GameObject_HasComponent(ObjectHandle p_Handle, MonoReflectionType* p_ComponentType)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = FindWithUUID(p_Handle);
			if (!entity)
			{
				KTN_CORE_ERROR("Invalid Entity UUID!");
				return false;
			}

			MonoType* managedType = mono_reflection_type_get_type(p_ComponentType);
			KTN_CORE_VERIFY(s_EntityHasComponentFuncs.find(managedType) != s_EntityHasComponentFuncs.end());
			return s_EntityHasComponentFuncs.at(managedType)(entity);
		}

		static ObjectHandle GameObject_FindWithTag(MonoString* p_Tag)
		{
			KTN_PROFILE_FUNCTION_LOW();

			auto tag = MonoStringToString(p_Tag);
			Entity entity = SceneManager::GetEntityByTag(tag);
			if (!entity)
				return { 0, 0 };

			return { entity.GetUUID(), entity.GetScene()->Handle, 0 };
		}

		static ObjectHandle GameObject_FindWithUUID(UUID p_ID)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_ID);
			if (!entity)
				return { 0, 0 };

			return { entity.GetUUID(), entity.GetScene()->Handle, 0 };
		}

		static bool GameObject_IsValid(ObjectHandle p_Handle)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = FindWithUUID(p_Handle);
			return entity ? true : false;
		}
		#pragma	endregion

		#pragma region SceneManager
		static int SceneManager_GetConfigLoadMode()
		{
			KTN_PROFILE_FUNCTION_LOW();

			return (int)SceneManager::GetConfig().Mode;
		}

		static void SceneManager_SetConfigLoadMode(int p_LoadMode)
		{
			KTN_PROFILE_FUNCTION_LOW();

			SceneManager::GetConfig().Mode = (LoadMode)p_LoadMode;
		}

		static bool SceneManager_LoadSceneByPath(MonoString* p_ScenePath, int p_Mode)
		{
			KTN_PROFILE_FUNCTION_LOW();

			auto str = MonoStringToString(p_ScenePath);
			auto path = Project::GetAssetFileSystemPath(str).string();

			auto handle = AssetManager::Get()->GetHandleByPath(path);

			return SceneManager::Load(handle, (LoadMode)p_Mode, true);
		}

		static bool SceneManager_LoadScene(AssetHandle p_SceneHandle, int p_Mode)
		{
			KTN_PROFILE_FUNCTION_LOW();

			return SceneManager::Load(p_SceneHandle, (LoadMode)p_Mode, true);
		}

		static void SceneManager_LoadSceneAsyncByPath(MonoString* p_ScenePath, int p_Mode)
		{
			KTN_PROFILE_FUNCTION_LOW();

			auto str = MonoStringToString(p_ScenePath);
			auto path = Project::GetAssetFileSystemPath(str).string();
			auto handle = AssetManager::Get()->GetHandleByPath(path);

			SceneManager::LoadAsync(handle, (LoadMode)p_Mode);
		}

		static void SceneManager_LoadSceneAsync(AssetHandle p_SceneHandle, int p_Mode)
		{
			KTN_PROFILE_FUNCTION_LOW();

			SceneManager::LoadAsync(p_SceneHandle, (LoadMode)p_Mode);
		}

		static void SceneManager_UnloadSceneByPath(MonoString* p_ScenePath)
		{
			KTN_PROFILE_FUNCTION_LOW();

			auto str = MonoStringToString(p_ScenePath);
			auto path = Project::GetAssetFileSystemPath(str).string();

			auto handle = AssetManager::Get()->GetHandleByPath(path);
			SceneManager::Unload(handle, true);
		}

		static void SceneManager_UnloadScene(AssetHandle p_SceneHandle)
		{
			KTN_PROFILE_FUNCTION_LOW();

			SceneManager::Unload(p_SceneHandle, true);
		}

		static void SceneManager_Pause(bool p_Value)
		{
			KTN_PROFILE_FUNCTION_LOW();

			SceneManager::Pause(p_Value);
		}

		static bool SceneManager_IsPaused()
		{
			KTN_PROFILE_FUNCTION_LOW();

			return SceneManager::IsPaused();
		}

		#pragma endregion

		#pragma region Scene

		static void Scene_Pause(AssetHandle p_Handle, bool p_Value)
		{
			auto scene = AssetManager::Get()->GetAsset<Scene>(p_Handle);
			KTN_CORE_VERIFY(scene, "Scene is nullptr!");

			scene->SetIsPaused(p_Value);
		}

		static bool Scene_IsPaused(AssetHandle p_Handle)
		{
			auto scene = AssetManager::Get()->GetAsset<Scene>(p_Handle);
			KTN_CORE_VERIFY(scene, "Scene is nullptr!");

			return scene->IsPaused();
		}

		static bool Scene_IsEntityValid(AssetHandle p_Handle, UUID p_EntityID)
		{
			auto scene = AssetManager::Get()->GetAsset<Scene>(p_Handle);
			KTN_CORE_VERIFY(scene, "Scene is nullptr!");

			if (p_EntityID == 0) return false;

			auto entt = scene->GetEntityByUUID(p_EntityID);

			return entt ? true : false;
		}

		static UUID Scene_GetEntityByTag(AssetHandle p_Handle, MonoString* p_Tag)
		{
			auto scene = AssetManager::Get()->GetAsset<Scene>(p_Handle);
			KTN_CORE_VERIFY(scene, "Scene is nullptr!");

			auto str = MonoStringToString(p_Tag);
			auto entt = scene->GetEntityByTag(str);

			return entt ? entt.GetUUID() : UUID(0);
		}

		#pragma endregion

		#pragma region TagComponent
		static MonoString* TagComponent_GetTag(UUID p_EntityID)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			if (!entity)
			{
				KTN_CORE_ERROR("Invalid Entity UUID!");
				return nullptr;
			}

			auto& tc = entity.GetComponent<TagComponent>();
			return ScriptEngine::CreateString(tc.Tag.c_str());
		}

		static void TagComponent_SetTag(UUID p_EntityID, MonoString* p_Tag)
		{
			KTN_PROFILE_FUNCTION_LOW();

			auto tag = MonoStringToString(p_Tag);
			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			if (!entity)
			{
				KTN_CORE_ERROR("Invalid Entity UUID!");
				return;
			}

			auto& tc = entity.GetComponent<TagComponent>();
			tc.Tag = tag;
		}
		#pragma endregion

		#pragma region TransformComponent
		static void TransformComponent_GetLocalTranslation(UUID p_EntityID, glm::vec3* p_Result)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TransformComponent>());

			*p_Result = entity.GetComponent<TransformComponent>().GetLocalTranslation();
		}

		static void TransformComponent_SetLocalTranslation(UUID p_EntityID, glm::vec3* p_Value)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TransformComponent>());

			auto& tc = entity.GetComponent<TransformComponent>();
			tc.SetLocalTranslation(*p_Value);
		}
		#pragma endregion

		#pragma region RuntimeComponent
		static bool RuntimeComponent_IsEnabled(UUID p_EntityID)
		{
			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			if (!entity)
			{
				KTN_CORE_ERROR("Invalid Entity UUID!");
				return false;
			}

			return entity.GetComponent<RuntimeComponent>().Enabled;
		}

		static bool RuntimeComponent_IsActive(UUID p_EntityID)
		{
			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			if (!entity)
			{
				KTN_CORE_ERROR("Invalid Entity UUID!");
				return false;
			}

			return entity.GetComponent<RuntimeComponent>().Active;
		}

		static void RuntimeComponent_SetEnabled(UUID p_EntityID, bool p_Value)
		{
			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			if (!entity)
			{
				KTN_CORE_ERROR("Invalid Entity UUID!");
				return;
			}

			auto& rc = entity.GetComponent<RuntimeComponent>();
			rc.Enabled = p_Value;
		}

		static void RuntimeComponent_SetActive(UUID p_EntityID, bool p_Value)
		{
			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			if (!entity)
			{
				KTN_CORE_ERROR("Invalid Entity UUID!");
				return;
			}

			auto& rc = entity.GetComponent<RuntimeComponent>();
			rc.Active = p_Value;
		}
		#pragma endregion

		#pragma region TextRendererComponent
		static MonoString* TextRendererComponent_GetString(UUID p_EntityID)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			return ScriptEngine::CreateString(tc.String.c_str());
		}

		static void TextRendererComponent_SetString(UUID p_EntityID, MonoString* p_String)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			tc.String = MonoStringToString(p_String);
		}

		static void TextRendererComponent_SetFont(UUID p_EntityID, MonoString* p_Path)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			std::string fontPath = MonoStringToString(p_Path);
			if (fontPath.empty())
				tc.Font = DFFont::GetDefault();
			else
			{
				tc.Font = AssetManager::Get()->GetHandleByPath(fontPath);
				KTN_CORE_VERIFY(tc.Font != 0, "Failed to load font from path: " + fontPath + ", Try importing the font first!");
			}
		}

		static MonoString* TextRendererComponent_GetFontPath(UUID p_EntityID)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			return ScriptEngine::CreateString(AssetManager::Get()->GetMetadata(tc.Font).FilePath.c_str());
		}

		static MonoString* TextRendererComponent_GetFontName(UUID p_EntityID)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			return ScriptEngine::CreateString(FileSystem::GetStem(AssetManager::Get()->GetMetadata(tc.Font).FilePath).c_str());
		}

		static void TextRendererComponent_GetColor(UUID p_EntityID, glm::vec4* p_Color)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			*p_Color = tc.Color;
		}

		static void TextRendererComponent_SetColor(UUID p_EntityID, glm::vec4* p_Color)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			tc.Color = *p_Color;
		}

		static void TextRendererComponent_GetBgColor(UUID p_EntityID, glm::vec4* p_Color)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			*p_Color = tc.BgColor;
		}

		static void TextRendererComponent_SetBgColor(UUID p_EntityID, glm::vec4* p_Color)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			tc.BgColor = *p_Color;
		}

		static void TextRendererComponent_GetCharBgColor(UUID p_EntityID, glm::vec4* p_Color)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			*p_Color = tc.CharBgColor;
		}

		static void TextRendererComponent_SetCharBgColor(UUID p_EntityID, glm::vec4* p_Color)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			tc.CharBgColor = *p_Color;
		}

		static bool TextRendererComponent_GetDrawBg(UUID p_EntityID)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			return tc.DrawBg;
		}

		static void TextRendererComponent_SetDrawBg(UUID p_EntityID, bool p_Enable)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			tc.DrawBg = p_Enable;
		}

		static float TextRendererComponent_GetKerning(UUID p_EntityID)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			return tc.Kerning;
		}

		static void TextRendererComponent_SetKerning(UUID p_EntityID, float p_Kerning)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			tc.Kerning = p_Kerning;
		}

		static float TextRendererComponent_GetLineSpacing(UUID p_EntityID)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			return tc.LineSpacing;
		}

		static void TextRendererComponent_SetLineSpacing(UUID p_EntityID, float p_LineSpacing)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			KTN_CORE_VERIFY(entity.HasComponent<TextRendererComponent>());
			auto& tc = entity.GetComponent<TextRendererComponent>();

			tc.LineSpacing = p_LineSpacing;
		}
		#pragma endregion

		#pragma region Input
		static bool Input_IsKeyPressed(KeyCode p_Keycode)
		{
			return Input::IsKeyPressed(p_Keycode);
		}

		static bool Input_IsKeyJustReleased(KeyCode p_Keycode)
		{
			return Input::IsKeyJustReleased(p_Keycode);
		}

		static bool Input_IsKeyJustPressed(KeyCode p_Keycode)
		{
			return Input::IsKeyJustPressed(p_Keycode);
		}

		static bool Input_IsKeyJustHeld(KeyCode p_Keycode)
		{
			return Input::IsKeyJustHeld(p_Keycode);
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
		#pragma endregion

		#pragma region Box2D

		static void B2_GetLinearVelocity(UUID p_EntityID, glm::vec2* p_OutVelocity)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);

			B2BodyID body = GetPhysicsBody2D(entity);
			KTN_CORE_VERIFY(body.Index != -1, "Entity does not have a physics body!");

			b2BodyId bodyId = { body.Index, body.World, body.Generation };
			auto velocity = b2Body_GetLinearVelocity(bodyId);
			p_OutVelocity->x = velocity.x;
			p_OutVelocity->y = velocity.y;
		}

		static void B2_SetLinearVelocity(UUID p_EntityID, glm::vec2* p_Velocity)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);
			
			B2BodyID body = GetPhysicsBody2D(entity);
			KTN_CORE_VERIFY(body.Index != -1, "Entity does not have a physics body!");

			b2BodyId bodyId = { body.Index, body.World, body.Generation };
			b2Body_SetLinearVelocity(bodyId, { p_Velocity->x, p_Velocity->y });
		}

		static float B2_GetAngularVelocity(UUID p_EntityID)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);

			B2BodyID body = GetPhysicsBody2D(entity);
			KTN_CORE_VERIFY(body.Index != -1, "Entity does not have a physics body!");

			b2BodyId bodyId = { body.Index, body.World, body.Generation };
			return b2Body_GetAngularVelocity(bodyId);
		}

		static void B2_SetAngularVelocity(UUID p_EntityID, float p_AngVelocity)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);

			B2BodyID body = GetPhysicsBody2D(entity);
			KTN_CORE_VERIFY(body.Index != -1, "Entity does not have a physics body!");

			b2BodyId bodyId = { body.Index, body.World, body.Generation };
			b2Body_SetAngularVelocity(bodyId, p_AngVelocity);
		}

		static void B2_ApplyForce(UUID p_EntityID, glm::vec2* p_Force)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);

			B2BodyID body = GetPhysicsBody2D(entity);
			KTN_CORE_VERIFY(body.Index != -1, "Entity does not have a physics body!");

			b2BodyId bodyId = { body.Index, body.World, body.Generation };
			b2Body_ApplyForceToCenter(bodyId, { p_Force->x, p_Force->y }, true);
		}

		static void B2_ApplyLinearImpulse(UUID p_EntityID, glm::vec2* p_Impulse)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);

			B2BodyID body = GetPhysicsBody2D(entity);
			KTN_CORE_VERIFY(body.Index != -1, "Entity does not have a physics body!");

			b2BodyId bodyId = { body.Index, body.World, body.Generation };
			b2Body_ApplyLinearImpulseToCenter(bodyId, { p_Impulse->x, p_Impulse->y }, true);
		}

		static void B2_ApplyAngularImpulse(UUID p_EntityID, float p_Impulse)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);

			B2BodyID body = GetPhysicsBody2D(entity);
			KTN_CORE_VERIFY(body.Index != -1, "Entity does not have a physics body!");

			b2BodyId bodyId = { body.Index, body.World, body.Generation };
			b2Body_ApplyAngularImpulse(bodyId, p_Impulse, true);
		}

		static void B2_ApplyTorque(UUID p_EntityID, float p_Torque)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);

			B2BodyID body = GetPhysicsBody2D(entity);
			KTN_CORE_VERIFY(body.Index != -1, "Entity does not have a physics body!");

			b2BodyId bodyId = { body.Index, body.World, body.Generation };
			b2Body_ApplyTorque(bodyId, p_Torque, true);
		}

		static void B2_GetGravity(UUID p_EntityID, glm::vec2* p_OutGravity)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);

			auto scene = entity.GetScene();
			if (!scene->GetSystemManager()->HasSystem<B2Physics>())
			{
				KTN_CORE_ERROR("The scene does not have the B2Physics system.");
				p_OutGravity->x = p_OutGravity->y = 0.0f;
			}

			auto gravity = scene->GetSystemManager()->GetSystem<B2Physics>()->GetGravity(entity);
			p_OutGravity->x = gravity.x;
			p_OutGravity->y = gravity.y;
		}

		static void B2_GetRealGravity(UUID p_EntityID, glm::vec2* p_OutGravity)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);

			auto scene = entity.GetScene();
			if (!scene->GetSystemManager()->HasSystem<B2Physics>())
			{
				KTN_CORE_ERROR("The scene does not have the B2Physics system.");
				p_OutGravity->x = p_OutGravity->y = 0.0f;
			}

			auto gravity = scene->GetSystemManager()->GetSystem<B2Physics>()->GetRealGravity();
			p_OutGravity->x = gravity.x;
			p_OutGravity->y = gravity.y;
		}

		#pragma endregion

		#pragma region CharacterBody2DComponent

		static void CharacterBody2DComponent_MoveAndSlide(UUID p_EntityID)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);

			auto* scene = entity.GetScene();
			if (scene->GetSystemManager()->HasSystem<B2Physics>())
				scene->GetSystemManager()->GetSystem<B2Physics>()->MoveAndSlide(entity);
		}

		static void CharacterBody2DComponent_MoveAndCollide(UUID p_EntityID)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);

			auto* scene = entity.GetScene();
			if (scene->GetSystemManager()->HasSystem<B2Physics>())
				scene->GetSystemManager()->GetSystem<B2Physics>()->MoveAndCollide(entity);
		}

		static bool CharacterBody2DComponent_IsOnFloor(UUID p_EntityID)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);

			if (!entity.HasComponent<CharacterBody2DComponent>())
				return false;

			return entity.GetComponent<CharacterBody2DComponent>().OnFloor;
		}

		static bool CharacterBody2DComponent_IsOnWall(UUID p_EntityID)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);

			if (!entity.HasComponent<CharacterBody2DComponent>())
				return false;

			return entity.GetComponent<CharacterBody2DComponent>().OnWall;
		}

		static bool CharacterBody2DComponent_IsOnCeiling(UUID p_EntityID)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = SceneManager::GetEntityByUUID(p_EntityID);
			KTN_CORE_VERIFY(entity);

			if (!entity.HasComponent<CharacterBody2DComponent>())
				return false;

			return entity.GetComponent<CharacterBody2DComponent>().OnCeiling;
		}

		#pragma endregion

		std::string DemangleToKTNClassName(const std::string& p_MangledName) 
		{
			KTN_PROFILE_FUNCTION_LOW();

			auto n3pos = p_MangledName.find("N3");
			if (n3pos == std::string::npos) 
				return p_MangledName; // no need to demangle

			size_t currentPos = n3pos + 5;
			std::string result = "KTN.";

			while (currentPos < p_MangledName.length()) 
			{
				size_t numStart = currentPos;
				size_t numEnd = p_MangledName.find_first_not_of("0123456789", numStart);
				int segmentLength = std::stoi(p_MangledName.substr(numStart, numEnd - numStart));

				size_t pos = numEnd + segmentLength;
				std::string segment = p_MangledName.substr(numEnd, segmentLength);
				if (pos < p_MangledName.length() && p_MangledName[pos] == 'E')
					return result + segment;

				currentPos = pos;
			}

			size_t numStart = n3pos + 5;
			size_t numEnd = p_MangledName.find_first_not_of("0123456789", numStart);
			if (numEnd != std::string::npos)
			{
				int length = std::stoi(p_MangledName.substr(numStart, numEnd - numStart));
				return result + p_MangledName.substr(numEnd, length);
			}

			return p_MangledName;
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

				std::string managedTypename = DemangleToKTNClassName(std::string("KTN.") + std::string(structName.data()));

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

		KTN_ADD_INTERNAL_CALL(GetScriptInstance);

		KTN_ADD_INTERNAL_CALL(Object_Instantiate);

		KTN_ADD_INTERNAL_CALL(AssetManager_FindWithPath);
		KTN_ADD_INTERNAL_CALL(AssetManager_IsAssetHandleValid);
		KTN_ADD_INTERNAL_CALL(AssetManager_IsAssetLoaded);

		KTN_ADD_INTERNAL_CALL(Time_GetTime);
		KTN_ADD_INTERNAL_CALL(Time_GetDeltaTime);

		KTN_ADD_INTERNAL_CALL(GameObject_HasComponent);
		KTN_ADD_INTERNAL_CALL(GameObject_FindWithTag);
		KTN_ADD_INTERNAL_CALL(GameObject_FindWithUUID);
		KTN_ADD_INTERNAL_CALL(GameObject_IsValid);

		KTN_ADD_INTERNAL_CALL(SceneManager_GetConfigLoadMode);
		KTN_ADD_INTERNAL_CALL(SceneManager_SetConfigLoadMode);
		KTN_ADD_INTERNAL_CALL(SceneManager_LoadSceneByPath);
		KTN_ADD_INTERNAL_CALL(SceneManager_LoadScene);
		KTN_ADD_INTERNAL_CALL(SceneManager_LoadSceneAsyncByPath);
		KTN_ADD_INTERNAL_CALL(SceneManager_LoadSceneAsync);
		KTN_ADD_INTERNAL_CALL(SceneManager_UnloadSceneByPath);
		KTN_ADD_INTERNAL_CALL(SceneManager_UnloadScene);
		KTN_ADD_INTERNAL_CALL(SceneManager_Pause);
		KTN_ADD_INTERNAL_CALL(SceneManager_IsPaused);

		KTN_ADD_INTERNAL_CALL(Scene_Pause);
		KTN_ADD_INTERNAL_CALL(Scene_IsPaused);
		KTN_ADD_INTERNAL_CALL(Scene_IsEntityValid);
		KTN_ADD_INTERNAL_CALL(Scene_GetEntityByTag);

		KTN_ADD_INTERNAL_CALL(TagComponent_GetTag);
		KTN_ADD_INTERNAL_CALL(TagComponent_SetTag);

		KTN_ADD_INTERNAL_CALL(TransformComponent_GetLocalTranslation);
		KTN_ADD_INTERNAL_CALL(TransformComponent_SetLocalTranslation);

		KTN_ADD_INTERNAL_CALL(RuntimeComponent_IsEnabled);
		KTN_ADD_INTERNAL_CALL(RuntimeComponent_IsActive);
		KTN_ADD_INTERNAL_CALL(RuntimeComponent_SetEnabled);
		KTN_ADD_INTERNAL_CALL(RuntimeComponent_SetActive);

		KTN_ADD_INTERNAL_CALL(TextRendererComponent_GetString);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_SetString);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_SetFont);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_GetFontPath);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_GetFontName);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_GetColor);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_SetColor);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_GetBgColor);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_SetBgColor);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_GetCharBgColor);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_SetCharBgColor);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_GetDrawBg);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_SetDrawBg);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_GetKerning);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_SetKerning);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_GetLineSpacing);
		KTN_ADD_INTERNAL_CALL(TextRendererComponent_SetLineSpacing);

		KTN_ADD_INTERNAL_CALL(Input_IsKeyPressed);
		KTN_ADD_INTERNAL_CALL(Input_IsKeyJustReleased);
		KTN_ADD_INTERNAL_CALL(Input_IsKeyJustPressed);
		KTN_ADD_INTERNAL_CALL(Input_IsKeyJustHeld);
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

		KTN_ADD_INTERNAL_CALL(B2_GetLinearVelocity);
		KTN_ADD_INTERNAL_CALL(B2_SetLinearVelocity);
		KTN_ADD_INTERNAL_CALL(B2_GetAngularVelocity);
		KTN_ADD_INTERNAL_CALL(B2_SetAngularVelocity);
		KTN_ADD_INTERNAL_CALL(B2_ApplyForce);
		KTN_ADD_INTERNAL_CALL(B2_ApplyLinearImpulse);
		KTN_ADD_INTERNAL_CALL(B2_ApplyAngularImpulse);
		KTN_ADD_INTERNAL_CALL(B2_ApplyTorque);
		KTN_ADD_INTERNAL_CALL(B2_GetGravity);
		KTN_ADD_INTERNAL_CALL(B2_GetRealGravity);

		KTN_ADD_INTERNAL_CALL(CharacterBody2DComponent_MoveAndSlide);
		KTN_ADD_INTERNAL_CALL(CharacterBody2DComponent_MoveAndCollide);
		KTN_ADD_INTERNAL_CALL(CharacterBody2DComponent_IsOnFloor);
		KTN_ADD_INTERNAL_CALL(CharacterBody2DComponent_IsOnWall);
		KTN_ADD_INTERNAL_CALL(CharacterBody2DComponent_IsOnCeiling);
	}

} // namespace KTN