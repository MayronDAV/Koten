#include "ktnpch.h"
#include "SceneSerializer.h"
#include "Koten/Asset/TextureImporter.h"
#include "Koten/Script/ScriptEngine.h"
#include "Koten/Project/Project.h"

// lib
#include <yaml-cpp/yaml.h>




namespace KTN
{
	namespace
	{
		#pragma region YAML

		template<typename Component>
		void ComponentSerializeIfExist(YAML::Emitter& p_Out, entt::registry& p_Registry, Entity p_Entity) {}

		template<typename Component>
		void ComponentDeserializeIfExist(YAML::Node& p_Data, entt::registry& p_Registry, Entity p_Entity) {}

		template<typename... Component>
		void ComponentSerialize(YAML::Emitter& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			(ComponentSerializeIfExist<Component>(p_Out, p_Registry, p_Entity), ...);
		}

		template<typename... Component>
		void ComponentDeserialize(YAML::Node& p_Data, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			(ComponentDeserializeIfExist<Component>(p_Data, p_Registry, p_Entity), ...);
		}

		#pragma endregion

		#pragma region BIN

		template<typename Component>
		void ComponentSerializeBinIfExist(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity) {}

		template<typename Component>
		void ComponentDeserializeBinIfExist(std::ifstream& p_Input, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity) {}

		template<typename... Component>
		void ComponentSerializeBin(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			int count = 0;
			auto countComponents = [&count, &p_Entity]<typename T>()
			{
				if (p_Entity.HasComponent<T>())
				{
					count++;
				}
			};
			(countComponents.template operator() < Component > (), ...);

			count = count > 0 ? count - 1 : 0; // Exclude the IDComponent
			p_Out.write(reinterpret_cast<const char*>(&count), sizeof(count));
			(ComponentSerializeBinIfExist<Component>(p_Out, p_Registry, p_Entity), ...);
		}

		template<typename... Component>
		void ComponentDeserializeBin(std::ifstream& p_Input, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			int count = 0;
			p_Input.read(reinterpret_cast<char*>(&count), sizeof(count));
			if (count <= 0)
			{
				KTN_CORE_WARN("ComponentDeserializeBin - No components to deserialize! Entity: ( Tag: {}, ID: {} )", p_Entity.GetTag(), (uint64_t)p_Entity.GetUUID());
				return;
			}

			for (int i = 0; i < count; i++)
			{
				auto current = Utils::ReadString(p_Input);
				(ComponentDeserializeBinIfExist<Component>(p_Input, current, p_Registry, p_Entity), ...);
			}
		}
		
		#pragma endregion

	} // namespace

	SceneSerializer::SceneSerializer(const Ref<Scene>& p_Scene)
		: m_Scene(p_Scene)
	{
	}

	void SceneSerializer::Serialize(const std::string& p_Filepath)
	{
		KTN_PROFILE_FUNCTION();

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << m_Scene->Handle;
		out << YAML::Key << "Configs" << YAML::Value << YAML::BeginSeq;
		{
			out << YAML::BeginMap;
			auto& config = m_Scene->GetConfig();
			out << YAML::Key << "UseB2Physics" << YAML::Value << config.UseB2Physics;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		for (auto entity : m_Scene->GetRegistry().view<entt::entity>())
		{
			auto entt = Entity{ entity, m_Scene.get() };
			if (!entt)
				continue;

			out << YAML::BeginMap;
			out << YAML::Key << "Entity" << YAML::Value << entt.GetUUID();

			ComponentSerialize<ALL_COMPONENTS>(out, m_Scene->GetRegistry(), entt);

			out << YAML::EndMap;
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(p_Filepath);
		fout << out.c_str();

		KTN_CORE_INFO("Scene serialized to path: {}", p_Filepath);
	}

	bool SceneSerializer::Deserialize(const std::string& p_Filepath)
	{
		KTN_PROFILE_FUNCTION();

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(p_Filepath);
		}
		catch (YAML::ParserException e)
		{
			KTN_CORE_ERROR("Failed to load .uyscene file '{0}'\n     {1}", p_Filepath, e.what());
			return false;
		}

		if (!data["Scene"])
			return false;

		auto sceneID = data["Scene"].as<AssetHandle>();
		KTN_CORE_INFO("Deserializing scene: (filename: '{}', id: '{}')", FileSystem::GetName(p_Filepath), (uint64_t)sceneID);
		if (sceneID != 0)
			m_Scene->Handle = sceneID;

		auto configs = data["Configs"];
		if (configs)
		{
			auto& config = m_Scene->GetConfig();
			for (auto value : configs)
			{
				config.UseB2Physics = value["UseB2Physics"].as<bool>();
			}
		}

		auto entts = data["Entities"];
		if (entts)
		{
			// TODO: Maybe find another way to load the entt hierarchy
			for (auto entt : entts)
			{
				auto enttID = entt["Entity"].as<uint64_t>();
				Entity deserializedEntt = m_Scene->CreateEntity((UUID)enttID, "Entity");
			}

			for (auto entt : entts)
			{
				auto enttID = entt["Entity"].as<uint64_t>();
				Entity deserializedEntt = m_Scene->GetEntityByUUID((UUID)enttID);

				ComponentDeserialize<ALL_COMPONENTS>(entt, m_Scene->GetRegistry(), deserializedEntt);
			}
		}

		return true;
	}

	void SceneSerializer::SerializeBin(std::ofstream& p_Out)
	{
		KTN_PROFILE_FUNCTION();

		const auto& enttMap = m_Scene->GetEntityMap();

		SceneConfig config = m_Scene->GetConfig();
		p_Out.write(reinterpret_cast<const char*>(&config), sizeof(config));

		auto size = (int)enttMap.size();
		p_Out.write(reinterpret_cast<const char*>(&size), sizeof(size));

		for (const auto& [uuid, entity] : enttMap)
		{
			p_Out.write(reinterpret_cast<const char*>(&uuid), sizeof(uuid));

			auto entt = Entity{ entity, m_Scene.get() };
			ComponentSerializeBin<ALL_COMPONENTS>(p_Out, m_Scene->GetRegistry(), entt);
		}
	}

	bool SceneSerializer::DeserializeBin(std::ifstream& p_In)
	{
		SceneConfig config;
		p_In.read(reinterpret_cast<char*>(&config), sizeof(config));
		m_Scene->GetConfig() = config;

		int size = 0;
		p_In.read(reinterpret_cast<char*>(&size), sizeof(size));

		for (int i = 0; i < size; i++)
		{
			UUID uuid;
			p_In.read(reinterpret_cast<char*>(&uuid), sizeof(uuid));

			Entity entt = m_Scene->CreateEntity(uuid);

			ComponentDeserializeBin<ALL_COMPONENTS>(p_In, m_Scene->GetRegistry(), entt);
		}

		return true;
	}

	// Serialize YAML
	namespace
	{
		#define ADD_KEY_VALUE(KeyName, KeyValue) p_Out << YAML::Key << KeyName << YAML::Value << KeyValue

		template<>
		void ComponentSerializeIfExist<TagComponent>(YAML::Emitter& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<TagComponent>())
				return;

			p_Out << YAML::Key << "TagComponent";
			p_Out << YAML::BeginMap;

			auto tag = p_Entity.GetTag();
			ADD_KEY_VALUE("Tag", tag);

			p_Out << YAML::EndMap;
		}

		template<>
		void ComponentSerializeIfExist<HierarchyComponent>(YAML::Emitter& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<HierarchyComponent>())
				return;

			p_Out << YAML::Key << "HierarchyComponent";
			p_Out << YAML::BeginMap;

			auto& comp = p_Entity.GetComponent<HierarchyComponent>();

			if (comp.Parent != entt::null)
			{
				auto entt = Entity{ comp.Parent, p_Entity.GetScene() };
				ADD_KEY_VALUE("Parent", (uint64_t)entt.GetUUID());
			}
			else
				ADD_KEY_VALUE("Parent", 0ul);

			if (comp.First != entt::null)
			{
				auto entt = Entity{ comp.First, p_Entity.GetScene() };
				ADD_KEY_VALUE("First", (uint64_t)entt.GetUUID());
			}
			else
				ADD_KEY_VALUE("First", 0ul);

			if (comp.Prev != entt::null)
			{
				auto entt = Entity{ comp.Prev, p_Entity.GetScene() };
				ADD_KEY_VALUE("Prev", (uint64_t)entt.GetUUID());
			}
			else
				ADD_KEY_VALUE("Prev", 0ul);

			if (comp.Next != entt::null)
			{
				auto entt = Entity{ comp.Next, p_Entity.GetScene() };
				ADD_KEY_VALUE("Next", (uint64_t)entt.GetUUID());
			}
			else
				ADD_KEY_VALUE("Next", 0ul);

			ADD_KEY_VALUE("ChildCount", comp.ChildCount);

			p_Out << YAML::EndMap;
		}

		template<>
		void ComponentSerializeIfExist<TransformComponent>(YAML::Emitter& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<TransformComponent>())
				return;

			p_Out << YAML::Key << "TransformComponent";
			p_Out << YAML::BeginMap;

			auto& comp = p_Entity.GetComponent<TransformComponent>();
			ADD_KEY_VALUE("Translation", comp.GetLocalTranslation());
			ADD_KEY_VALUE("Rotation", comp.GetLocalRotation());
			ADD_KEY_VALUE("Scale", comp.GetLocalScale());

			p_Out << YAML::EndMap;
		}

		template<>
		void ComponentSerializeIfExist<CameraComponent>(YAML::Emitter& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<CameraComponent>())
				return;

			p_Out << YAML::Key << "CameraComponent";
			p_Out << YAML::BeginMap;

			auto& comp = p_Entity.GetComponent<CameraComponent>();
			ADD_KEY_VALUE("Primary", comp.Primary);
			ADD_KEY_VALUE("ClearColor", comp.ClearColor);
			ADD_KEY_VALUE("Width", comp.Camera.GetViewportWidth());
			ADD_KEY_VALUE("Height", comp.Camera.GetViewportHeight());
			ADD_KEY_VALUE("IsOrthographic", comp.Camera.IsOrthographic());
			ADD_KEY_VALUE("IsAspectRatioFixed", comp.Camera.IsAspectRatioFixed());
			ADD_KEY_VALUE("FOV", comp.Camera.GetFOV());
			ADD_KEY_VALUE("Far", comp.Camera.GetFar());
			ADD_KEY_VALUE("Near", comp.Camera.GetNear());
			ADD_KEY_VALUE("Scale", comp.Camera.GetScale());
			ADD_KEY_VALUE("Zoom", comp.Camera.GetZoom());

			p_Out << YAML::EndMap;
		}

		template<>
		void ComponentSerializeIfExist<SpriteComponent>(YAML::Emitter& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<SpriteComponent>())
				return;

			p_Out << YAML::Key << "SpriteComponent";
			p_Out << YAML::BeginMap;

			auto& comp = p_Entity.GetComponent<SpriteComponent>();
			ADD_KEY_VALUE("Type", (int)comp.Type);
			ADD_KEY_VALUE("Color", comp.Color);
			ADD_KEY_VALUE("Texture", comp.Texture);
			ADD_KEY_VALUE("Thickness", comp.Thickness);
			ADD_KEY_VALUE("Fade", comp.Fade);
			ADD_KEY_VALUE("Size", comp.Size);
			ADD_KEY_VALUE("BySize", comp.BySize);
			ADD_KEY_VALUE("Offset", comp.Offset);
			ADD_KEY_VALUE("Scale", comp.Scale);

			p_Out << YAML::EndMap;
		}

		template<>
		void ComponentSerializeIfExist<LineRendererComponent>(YAML::Emitter& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<LineRendererComponent>())
				return;

			p_Out << YAML::Key << "LineRendererComponent";
			p_Out << YAML::BeginMap;

			auto& comp = p_Entity.GetComponent<LineRendererComponent>();
			ADD_KEY_VALUE("Color", comp.Color);
			ADD_KEY_VALUE("Width", comp.Width);
			ADD_KEY_VALUE("Primitive", comp.Primitive);
			ADD_KEY_VALUE("Start", comp.Start);
			ADD_KEY_VALUE("End", comp.End);

			p_Out << YAML::EndMap;
		}

		template<>
		void ComponentSerializeIfExist<TextRendererComponent>(YAML::Emitter& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<TextRendererComponent>())
				return;

			p_Out << YAML::Key << "TextRendererComponent";
			p_Out << YAML::BeginMap;

			auto& comp = p_Entity.GetComponent<TextRendererComponent>();
			ADD_KEY_VALUE("String", comp.String);
			ADD_KEY_VALUE("Font", comp.Font);
			ADD_KEY_VALUE("Color", comp.Color);
			ADD_KEY_VALUE("BgColor", comp.BgColor);
			ADD_KEY_VALUE("CharBgColor", comp.CharBgColor);

			ADD_KEY_VALUE("DrawBg", comp.DrawBg);
			ADD_KEY_VALUE("Kerning", comp.Kerning);
			ADD_KEY_VALUE("LineSpacing", comp.LineSpacing);

			p_Out << YAML::EndMap;
		}

		template<>
		void ComponentSerializeIfExist<Rigidbody2DComponent>(YAML::Emitter& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<Rigidbody2DComponent>())
				return;

			p_Out << YAML::Key << "Rigidbody2DComponent";
			p_Out << YAML::BeginMap;

			auto& comp = p_Entity.GetComponent<Rigidbody2DComponent>();
			ADD_KEY_VALUE("Type", (int)comp.Type);
			ADD_KEY_VALUE("FixedRotation", comp.FixedRotation);
			ADD_KEY_VALUE("GravityScale", comp.GravityScale);

			p_Out << YAML::EndMap;
		}

		template<>
		void ComponentSerializeIfExist<BoxCollider2DComponent>(YAML::Emitter& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<BoxCollider2DComponent>())
				return;

			p_Out << YAML::Key << "BoxCollider2DComponent";
			p_Out << YAML::BeginMap;

			auto& comp = p_Entity.GetComponent<BoxCollider2DComponent>();

			ADD_KEY_VALUE("IsTrigger", comp.IsTrigger);
			ADD_KEY_VALUE("Offset", comp.Offset);
			ADD_KEY_VALUE("Size", comp.Size);
			ADD_KEY_VALUE("Density", comp.Density);
			ADD_KEY_VALUE("Friction", comp.Friction);
			ADD_KEY_VALUE("Restitution", comp.Restitution);
			ADD_KEY_VALUE("RestitutionThreshold", comp.RestitutionThreshold);

			p_Out << YAML::EndMap;
		}

		template<>
		void ComponentSerializeIfExist<CircleCollider2DComponent>(YAML::Emitter& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();
			if (!p_Entity.HasComponent<CircleCollider2DComponent>())
				return;

			p_Out << YAML::Key << "CircleCollider2DComponent";
			p_Out << YAML::BeginMap;

			auto& comp = p_Entity.GetComponent<CircleCollider2DComponent>();

			ADD_KEY_VALUE("IsTrigger", comp.IsTrigger);
			ADD_KEY_VALUE("Offset", comp.Offset);
			ADD_KEY_VALUE("Radius", comp.Radius);
			ADD_KEY_VALUE("Density", comp.Density);
			ADD_KEY_VALUE("Friction", comp.Friction);
			ADD_KEY_VALUE("Restitution", comp.Restitution);
			ADD_KEY_VALUE("RestitutionThreshold", comp.RestitutionThreshold);

			p_Out << YAML::EndMap;
		}

		template<>
		void ComponentSerializeIfExist<ScriptComponent>(YAML::Emitter& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			#define WRITE_SCRIPT_FIELD(FieldType, Type)           \
				case ScriptFieldType::FieldType:          \
					p_Out << scriptField.GetValue<Type>();  \
					break

			if (!p_Entity.HasComponent<ScriptComponent>())
				return;

			p_Out << YAML::Key << "ScriptComponent";
			p_Out << YAML::BeginMap;

			auto& comp = p_Entity.GetComponent<ScriptComponent>();
			ADD_KEY_VALUE("FullClassName", comp.FullClassName);

			Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(comp.FullClassName);
			const auto& fields = entityClass->GetFields();
			if (fields.size() > 0)
			{
				p_Out << YAML::Key << "ScriptFields" << YAML::Value;
				auto& entityFields = ScriptEngine::GetScriptFieldMap(p_Entity);
				p_Out << YAML::BeginSeq;
				for (const auto& [name, field] : fields)
				{
					if (entityFields.find(name) == entityFields.end())
						continue;

					if (field.IsPrivate && !field.Serialize)
						continue;

					p_Out << YAML::BeginMap; // ScriptField
					ADD_KEY_VALUE("Name", name);
					ADD_KEY_VALUE("Type", Utils::ScriptFieldTypeToString(field.Type));

					p_Out << YAML::Key << "Data" << YAML::Value;
					ScriptFieldInstance& scriptField = entityFields.at(name);

					switch (field.Type)
					{
						WRITE_SCRIPT_FIELD(Float, float);
						WRITE_SCRIPT_FIELD(Double, double);
						WRITE_SCRIPT_FIELD(Bool, bool);
						WRITE_SCRIPT_FIELD(Char, char);
						WRITE_SCRIPT_FIELD(Byte, int8_t);
						WRITE_SCRIPT_FIELD(Short, int16_t);
						WRITE_SCRIPT_FIELD(Int, int32_t);
						WRITE_SCRIPT_FIELD(Long, int64_t);
						WRITE_SCRIPT_FIELD(UByte, uint8_t);
						WRITE_SCRIPT_FIELD(UShort, uint16_t);
						WRITE_SCRIPT_FIELD(UInt, uint32_t);
						WRITE_SCRIPT_FIELD(ULong, uint64_t);
						WRITE_SCRIPT_FIELD(Vector2, glm::vec2);
						WRITE_SCRIPT_FIELD(Vector3, glm::vec3);
						WRITE_SCRIPT_FIELD(Vector4, glm::vec4);
						WRITE_SCRIPT_FIELD(Entity, UUID);
					}
					p_Out << YAML::EndMap; // ScriptFields
				}
				p_Out << YAML::EndSeq;
			}

			p_Out << YAML::EndMap;

			#undef WRITE_SCRIPT_FIELD
		}

	} // namespace

	// Serialize BIN
	namespace
	{
		template<>
		void ComponentSerializeBinIfExist<TagComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<TagComponent>())
				return;

			Utils::WriteString(p_Out, "TagComponent");

			Utils::WriteString(p_Out, p_Entity.GetTag());
		}

		template<>
		void ComponentSerializeBinIfExist<HierarchyComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<HierarchyComponent>())
				return;

			Utils::WriteString(p_Out, "HierarchyComponent");

			auto& comp = p_Entity.GetComponent<HierarchyComponent>();

			UUID null = 0ul;

			if (comp.Parent != entt::null)
			{
				auto entt = Entity{ comp.Parent, p_Entity.GetScene() };
				auto uuid = entt.GetUUID();
				p_Out.write(reinterpret_cast<const char*>(&uuid), sizeof(UUID));
			}
			else
				p_Out.write(reinterpret_cast<const char*>(&null), sizeof(UUID));

			if (comp.First != entt::null)
			{
				auto entt = Entity{ comp.First, p_Entity.GetScene() };
				auto uuid = entt.GetUUID();
				p_Out.write(reinterpret_cast<const char*>(&uuid), sizeof(UUID));
			}
			else
				p_Out.write(reinterpret_cast<const char*>(&null), sizeof(UUID));

			if (comp.Prev != entt::null)
			{
				auto entt = Entity{ comp.Prev, p_Entity.GetScene() };
				auto uuid = entt.GetUUID();
				p_Out.write(reinterpret_cast<const char*>(&uuid), sizeof(UUID));
			}
			else
				p_Out.write(reinterpret_cast<const char*>(&null), sizeof(UUID));

			if (comp.Next != entt::null)
			{
				auto entt = Entity{ comp.Next, p_Entity.GetScene() };
				auto uuid = entt.GetUUID();
				p_Out.write(reinterpret_cast<const char*>(&uuid), sizeof(UUID));
			}
			else
				p_Out.write(reinterpret_cast<const char*>(&null), sizeof(UUID));

			p_Out.write(reinterpret_cast<const char*>(&comp.ChildCount), sizeof(comp.ChildCount));
		}

		template<>
		void ComponentSerializeBinIfExist<TransformComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<TransformComponent>())
				return;

			Utils::WriteString(p_Out, "TransformComponent");

			auto& comp = p_Entity.GetComponent<TransformComponent>();

			glm::vec3 translation = comp.GetLocalTranslation();
			p_Out.write(reinterpret_cast<const char*>(&translation), sizeof(translation));
			glm::vec3 rotation = comp.GetLocalRotation();
			p_Out.write(reinterpret_cast<const char*>(&rotation), sizeof(rotation));
			glm::vec3 scale = comp.GetLocalScale();
			p_Out.write(reinterpret_cast<const char*>(&scale), sizeof(scale));
		}

		template<>
		void ComponentSerializeBinIfExist<CameraComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<CameraComponent>())
				return;

			Utils::WriteString(p_Out, "CameraComponent");

			auto& comp = p_Entity.GetComponent<CameraComponent>();

			p_Out.write(reinterpret_cast<const char*>(&comp.Primary), sizeof(comp.Primary));
			p_Out.write(reinterpret_cast<const char*>(&comp.ClearColor), sizeof(comp.ClearColor));
			uint32_t width = comp.Camera.GetViewportWidth();
			p_Out.write(reinterpret_cast<const char*>(&width), sizeof(width));
			uint32_t height = comp.Camera.GetViewportHeight();
			p_Out.write(reinterpret_cast<const char*>(&height), sizeof(height));
			bool isOrthographic = comp.Camera.IsOrthographic();
			p_Out.write(reinterpret_cast<const char*>(&isOrthographic), sizeof(isOrthographic));
			bool isAspectRatioFixed = comp.Camera.IsAspectRatioFixed();
			p_Out.write(reinterpret_cast<const char*>(&isAspectRatioFixed), sizeof(isAspectRatioFixed));
			float fov = comp.Camera.GetFOV();
			p_Out.write(reinterpret_cast<const char*>(&fov), sizeof(fov));
			float farZ = comp.Camera.GetFar();
			p_Out.write(reinterpret_cast<const char*>(&farZ), sizeof(farZ));
			float nearZ = comp.Camera.GetNear();
			p_Out.write(reinterpret_cast<const char*>(&nearZ), sizeof(nearZ));
			float scale = comp.Camera.GetScale();
			p_Out.write(reinterpret_cast<const char*>(&scale), sizeof(scale));
			float zoom = comp.Camera.GetZoom();
			p_Out.write(reinterpret_cast<const char*>(&zoom), sizeof(zoom));
		}

		template<>
		void ComponentSerializeBinIfExist<SpriteComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<SpriteComponent>())
				return;

			Utils::WriteString(p_Out, "SpriteComponent");

			auto& comp = p_Entity.GetComponent<SpriteComponent>();

			p_Out.write(reinterpret_cast<const char*>(&comp.Type), sizeof(comp.Type));
			p_Out.write(reinterpret_cast<const char*>(&comp.Color), sizeof(comp.Color));
			p_Out.write(reinterpret_cast<const char*>(&comp.Texture), sizeof(comp.Texture));
			p_Out.write(reinterpret_cast<const char*>(&comp.Thickness), sizeof(comp.Thickness));
			p_Out.write(reinterpret_cast<const char*>(&comp.Fade), sizeof(comp.Fade));
			p_Out.write(reinterpret_cast<const char*>(&comp.Size), sizeof(comp.Size));
			p_Out.write(reinterpret_cast<const char*>(&comp.BySize), sizeof(comp.BySize));
			p_Out.write(reinterpret_cast<const char*>(&comp.Offset), sizeof(comp.Offset));
			p_Out.write(reinterpret_cast<const char*>(&comp.Scale), sizeof(comp.Scale));
		}

		template<>
		void ComponentSerializeBinIfExist<LineRendererComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<LineRendererComponent>())
				return;

			Utils::WriteString(p_Out, "LineRendererComponent");

			auto& comp = p_Entity.GetComponent<LineRendererComponent>();

			p_Out.write(reinterpret_cast<const char*>(&comp.Color), sizeof(comp.Color));
			p_Out.write(reinterpret_cast<const char*>(&comp.Width), sizeof(comp.Width));
			p_Out.write(reinterpret_cast<const char*>(&comp.Primitive), sizeof(comp.Primitive));
			p_Out.write(reinterpret_cast<const char*>(&comp.Start), sizeof(comp.Start));
			p_Out.write(reinterpret_cast<const char*>(&comp.End), sizeof(comp.End));
		}

		template<>
		void ComponentSerializeBinIfExist<TextRendererComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<TextRendererComponent>())
				return;

			Utils::WriteString(p_Out, "TextRendererComponent");

			auto& comp = p_Entity.GetComponent<TextRendererComponent>();

			Utils::WriteString(p_Out, comp.String);

			p_Out.write(reinterpret_cast<const char*>(&comp.Font), sizeof(comp.Font));
			p_Out.write(reinterpret_cast<const char*>(&comp.Color), sizeof(comp.Color));
			p_Out.write(reinterpret_cast<const char*>(&comp.BgColor), sizeof(comp.BgColor));
			p_Out.write(reinterpret_cast<const char*>(&comp.CharBgColor), sizeof(comp.CharBgColor));
			p_Out.write(reinterpret_cast<const char*>(&comp.DrawBg), sizeof(comp.DrawBg));
			p_Out.write(reinterpret_cast<const char*>(&comp.Kerning), sizeof(comp.Kerning));
			p_Out.write(reinterpret_cast<const char*>(&comp.LineSpacing), sizeof(comp.LineSpacing));
		}

		template<>
		void ComponentSerializeBinIfExist<Rigidbody2DComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<Rigidbody2DComponent>())
				return;

			Utils::WriteString(p_Out, "Rigidbody2DComponent");

			auto& comp = p_Entity.GetComponent<Rigidbody2DComponent>();

			p_Out.write(reinterpret_cast<const char*>(&comp.Type), sizeof(comp.Type));
			p_Out.write(reinterpret_cast<const char*>(&comp.FixedRotation), sizeof(comp.FixedRotation));
			p_Out.write(reinterpret_cast<const char*>(&comp.GravityScale), sizeof(comp.GravityScale));
		}

		template<>
		void ComponentSerializeBinIfExist<BoxCollider2DComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<BoxCollider2DComponent>())
				return;

			Utils::WriteString(p_Out, "BoxCollider2DComponent");

			auto& comp = p_Entity.GetComponent<BoxCollider2DComponent>();

			p_Out.write(reinterpret_cast<const char*>(&comp.IsTrigger), sizeof(comp.IsTrigger));
			p_Out.write(reinterpret_cast<const char*>(&comp.Offset), sizeof(comp.Offset));
			p_Out.write(reinterpret_cast<const char*>(&comp.Size), sizeof(comp.Size));
			p_Out.write(reinterpret_cast<const char*>(&comp.Density), sizeof(comp.Density));
			p_Out.write(reinterpret_cast<const char*>(&comp.Friction), sizeof(comp.Friction));
			p_Out.write(reinterpret_cast<const char*>(&comp.Restitution), sizeof(comp.Restitution));
			p_Out.write(reinterpret_cast<const char*>(&comp.RestitutionThreshold), sizeof(comp.RestitutionThreshold));
		}

		template<>
		void ComponentSerializeBinIfExist<CircleCollider2DComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();
			if (!p_Entity.HasComponent<CircleCollider2DComponent>())
				return;

			Utils::WriteString(p_Out, "CircleCollider2DComponent");

			auto& comp = p_Entity.GetComponent<CircleCollider2DComponent>();

			p_Out.write(reinterpret_cast<const char*>(&comp.IsTrigger), sizeof(comp.IsTrigger));
			p_Out.write(reinterpret_cast<const char*>(&comp.Offset), sizeof(comp.Offset));
			p_Out.write(reinterpret_cast<const char*>(&comp.Radius), sizeof(comp.Radius));
			p_Out.write(reinterpret_cast<const char*>(&comp.Density), sizeof(comp.Density));
			p_Out.write(reinterpret_cast<const char*>(&comp.Friction), sizeof(comp.Friction));
			p_Out.write(reinterpret_cast<const char*>(&comp.Restitution), sizeof(comp.Restitution));
			p_Out.write(reinterpret_cast<const char*>(&comp.RestitutionThreshold), sizeof(comp.RestitutionThreshold));
		}

		template<>
		void ComponentSerializeBinIfExist<ScriptComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			#define WRITE_SCRIPT_FIELD(FieldType, Type)									\
				case ScriptFieldType::FieldType:										\
				{																		\
					auto value = scriptField.GetValue<Type>();							\
					p_Out.write(reinterpret_cast<const char*>(&value), sizeof(Type));	\
				}																		\
				break

			if (!p_Entity.HasComponent<ScriptComponent>())
				return;

			Utils::WriteString(p_Out, "ScriptComponent");

			auto& comp = p_Entity.GetComponent<ScriptComponent>();

			Utils::WriteString(p_Out, comp.FullClassName);

			Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(comp.FullClassName);
			const auto& fields = entityClass->GetFields();

			if (fields.size() > 0)
			{
				auto& entityFields = ScriptEngine::GetScriptFieldMap(p_Entity);

				int size = 0;
				for (const auto& [name, field] : fields)
				{
					if (entityFields.find(name) == entityFields.end())
						continue;

					if (field.IsPrivate && !field.Serialize)
						continue;

					size++;
				}
				p_Out.write(reinterpret_cast<const char*>(&size), sizeof(size));

				for (const auto& [name, field] : fields)
				{
					if (entityFields.find(name) == entityFields.end())
						continue;

					if (field.IsPrivate && !field.Serialize)
						continue;

					Utils::WriteString(p_Out, name);

					p_Out.write(reinterpret_cast<const char*>(&field.Type), sizeof(ScriptFieldType));

					ScriptFieldInstance& scriptField = entityFields.at(name);

					switch (field.Type)
					{
						WRITE_SCRIPT_FIELD(Float, float);
						WRITE_SCRIPT_FIELD(Double, double);
						WRITE_SCRIPT_FIELD(Bool, bool);
						WRITE_SCRIPT_FIELD(Char, char);
						WRITE_SCRIPT_FIELD(Byte, int8_t);
						WRITE_SCRIPT_FIELD(Short, int16_t);
						WRITE_SCRIPT_FIELD(Int, int32_t);
						WRITE_SCRIPT_FIELD(Long, int64_t);
						WRITE_SCRIPT_FIELD(UByte, uint8_t);
						WRITE_SCRIPT_FIELD(UShort, uint16_t);
						WRITE_SCRIPT_FIELD(UInt, uint32_t);
						WRITE_SCRIPT_FIELD(ULong, uint64_t);
						WRITE_SCRIPT_FIELD(Vector2, glm::vec2);
						WRITE_SCRIPT_FIELD(Vector3, glm::vec3);
						WRITE_SCRIPT_FIELD(Vector4, glm::vec4);
						WRITE_SCRIPT_FIELD(Entity, UUID);
					}
				}
			}

			#undef WRITE_SCRIPT_FIELD
		}

	} // namespace
	
	// Deserialize YAML
	namespace
	{
		template<>
		void ComponentDeserializeIfExist<TagComponent>(YAML::Node& p_Data, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			std::string tag = "Entity";
			auto tagComp = p_Data["TagComponent"];
			if (tagComp)
				tag = tagComp["Tag"].as<std::string>();

			p_Entity.GetComponent<TagComponent>().Tag = tag;
		}

		template<>
		void ComponentDeserializeIfExist<HierarchyComponent>(YAML::Node& p_Data, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			auto hierarchyComp = p_Data["HierarchyComponent"];
			if (!hierarchyComp) return;

			auto scene = p_Entity.GetScene();

			auto parent = hierarchyComp["Parent"].as<uint64_t>();
			auto first = hierarchyComp["First"].as<uint64_t>();
			auto prev = hierarchyComp["Prev"].as<uint64_t>();
			auto next = hierarchyComp["Next"].as<uint64_t>();

			auto& comp = p_Entity.AddOrReplaceComponent<HierarchyComponent>();
			comp.Parent = parent != 0ul ? scene->GetEntityByUUID(parent).GetHandle() : entt::null;
			comp.First = first != 0ul ? scene->GetEntityByUUID(first).GetHandle() : entt::null;
			comp.Prev = prev != 0ul ? scene->GetEntityByUUID(prev).GetHandle() : entt::null;
			comp.Next = next != 0ul ? scene->GetEntityByUUID(next).GetHandle() : entt::null;
			comp.ChildCount = hierarchyComp["ChildCount"].as<uint32_t>();
		}

		template<>
		void ComponentDeserializeIfExist<TransformComponent>(YAML::Node& p_Data, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			auto transformComp = p_Data["TransformComponent"];
			if (transformComp)
			{
				auto translation = transformComp["Translation"].as<glm::vec3>();
				auto scale = transformComp["Scale"].as<glm::vec3>();
				auto rotation = transformComp["Rotation"].as<glm::vec3>();

				p_Entity.AddComponent<TransformComponent>(translation, scale, rotation);
			}
		}

		template<>
		void ComponentDeserializeIfExist<CameraComponent>(YAML::Node& p_Data, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			auto cameraComp = p_Data["CameraComponent"];
			if (cameraComp)
			{
				auto primary = cameraComp["Primary"].as<bool>();
				auto clearColor = cameraComp["ClearColor"].as<glm::vec4>();
				auto width = cameraComp["Width"].as<uint32_t>();
				auto height = cameraComp["Height"].as<uint32_t>();
				auto isOrthographic = cameraComp["IsOrthographic"].as<bool>();
				auto isAspectRatioFixed = cameraComp["IsAspectRatioFixed"].as<bool>();
				auto fov = cameraComp["FOV"].as<float>();
				auto farz = cameraComp["Far"].as<float>();
				auto nearz = cameraComp["Near"].as<float>();
				auto scale = cameraComp["Scale"].as<float>();
				auto zoom = cameraComp["Zoom"].as<float>();

				auto& comp = p_Entity.AddComponent<CameraComponent>();
				comp.Primary = primary;
				comp.ClearColor = clearColor;
				comp.Camera.SetViewportSize(width, height);
				comp.Camera.SetIsOrthographic(isOrthographic);
				comp.Camera.SetFixAspectRatio(isAspectRatioFixed);
				comp.Camera.SetFOV(fov);
				comp.Camera.SetFar(farz);
				comp.Camera.SetNear(nearz);
				comp.Camera.SetScale(scale);
				comp.Camera.SetZoom(zoom);
				comp.Camera.OnUpdate();
			}
		}

		template<>
		void ComponentDeserializeIfExist<SpriteComponent>(YAML::Node& p_Data, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			auto spriteComp = p_Data["SpriteComponent"];
			if (spriteComp)
			{
				auto& comp = p_Entity.AddComponent<SpriteComponent>();
				comp.Color = spriteComp["Color"].as<glm::vec4>();
				comp.Type = (RenderType2D)spriteComp["Type"].as<int>();
				comp.Texture = spriteComp["Texture"].as<AssetHandle>();
				comp.Thickness = spriteComp["Thickness"].as<float>();
				comp.Fade = spriteComp["Fade"].as<float>();
				comp.Size = spriteComp["Size"].as<glm::vec2>();
				comp.BySize = spriteComp["BySize"].as<bool>();
				comp.Offset = spriteComp["Offset"].as<glm::vec2>();
				comp.Scale = spriteComp["Scale"].as<glm::vec2>();
			}
		}

		template<>
		void ComponentDeserializeIfExist<LineRendererComponent>(YAML::Node& p_Data, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			auto lineComp = p_Data["LineRendererComponent"];
			if (!lineComp) return;

			auto& comp = p_Entity.AddOrReplaceComponent<LineRendererComponent>();

			comp.Color = lineComp["Color"].as<glm::vec4>();
			comp.Width = lineComp["Width"].as<float>();
			comp.Primitive = lineComp["Primitive"].as<bool>();
			comp.Start = lineComp["Start"].as<glm::vec3>();
			comp.End = lineComp["End"].as<glm::vec3>();
		}

		template<>
		void ComponentDeserializeIfExist<TextRendererComponent>(YAML::Node& p_Data, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			auto textComp = p_Data["TextRendererComponent"];
			if (!textComp) return;

			auto& comp = p_Entity.AddOrReplaceComponent<TextRendererComponent>();

			comp.String = textComp["String"].as<std::string>();
			comp.Font = textComp["Font"].as<AssetHandle>();

			comp.Color = textComp["Color"].as<glm::vec4>();
			comp.BgColor = textComp["BgColor"].as<glm::vec4>();
			comp.CharBgColor = textComp["CharBgColor"].as<glm::vec4>();

			comp.DrawBg = textComp["DrawBg"].as<bool>();
			comp.Kerning = textComp["Kerning"].as<float>();
			comp.LineSpacing = textComp["LineSpacing"].as<float>();
		}

		template<>
		void ComponentDeserializeIfExist<Rigidbody2DComponent>(YAML::Node& p_Data, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			auto rigidComp = p_Data["Rigidbody2DComponent"];
			if (!rigidComp) return;

			auto& comp = p_Entity.AddOrReplaceComponent<Rigidbody2DComponent>();

			comp.Type = (Rigidbody2DComponent::BodyType)rigidComp["Type"].as<int>();
			comp.FixedRotation = rigidComp["FixedRotation"].as<bool>();
			comp.GravityScale = rigidComp["GravityScale"].as<float>();
		}

		template<>
		void ComponentDeserializeIfExist<BoxCollider2DComponent>(YAML::Node& p_Data, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			auto boxComp = p_Data["BoxCollider2DComponent"];
			if (!boxComp) return;

			auto& comp = p_Entity.AddOrReplaceComponent<BoxCollider2DComponent>();

			comp.IsTrigger = boxComp["IsTrigger"].as<bool>();
			comp.Offset = boxComp["Offset"].as<glm::vec2>();
			comp.Size = boxComp["Size"].as<glm::vec2>();
			comp.Density = boxComp["Density"].as<float>();
			comp.Friction = boxComp["Friction"].as<float>();
			comp.Restitution = boxComp["Restitution"].as<float>();
			comp.RestitutionThreshold = boxComp["RestitutionThreshold"].as<float>();
		}

		template<>
		void ComponentDeserializeIfExist<CircleCollider2DComponent>(YAML::Node& p_Data, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			auto circleComp = p_Data["CircleCollider2DComponent"];
			if (!circleComp) return;

			auto& comp = p_Entity.AddOrReplaceComponent<CircleCollider2DComponent>();

			comp.IsTrigger = circleComp["IsTrigger"].as<bool>();
			comp.Offset = circleComp["Offset"].as<glm::vec2>();
			comp.Radius = circleComp["Radius"].as<float>();
			comp.Density = circleComp["Density"].as<float>();
			comp.Friction = circleComp["Friction"].as<float>();
			comp.Restitution = circleComp["Restitution"].as<float>();
			comp.RestitutionThreshold = circleComp["RestitutionThreshold"].as<float>();
		}

		template<>
		void ComponentDeserializeIfExist<ScriptComponent>(YAML::Node& p_Data, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			#define READ_SCRIPT_FIELD(FieldType, Type)             \
				case ScriptFieldType::FieldType:                   \
				{                                                  \
					Type data = scriptField["Data"].as<Type>();    \
					fieldInstance.SetValue(data);                  \
					break;                                         \
				}

			std::string fullClassName = "";
			auto comp = p_Data["ScriptComponent"];
			if (comp)
			{
				auto& sc = p_Entity.AddOrReplaceComponent<ScriptComponent>();
				sc.FullClassName = comp["FullClassName"].as<std::string>();

				auto scriptFields = comp["ScriptFields"];
				if (scriptFields)
				{
					Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(sc.FullClassName);
					KTN_CORE_ASSERT(entityClass);
					const auto& fields = entityClass->GetFields();
					auto& entityFields = ScriptEngine::GetScriptFieldMap(p_Entity);

					for (auto scriptField : scriptFields)
					{
						std::string name = scriptField["Name"].as<std::string>();
						std::string typeString = scriptField["Type"].as<std::string>();
						ScriptFieldType type = Utils::ScriptFieldTypeFromString(typeString);

						ScriptFieldInstance& fieldInstance = entityFields[name];

						if (fields.find(name) == fields.end())
						{
							KTN_CORE_WARN("Script field '{}' not found in class '{}'. Skipping...", name, sc.FullClassName);
							continue;
						}

						fieldInstance.Field = fields.at(name);

						switch (type)
						{
							READ_SCRIPT_FIELD(Float, float);
							READ_SCRIPT_FIELD(Double, double);
							READ_SCRIPT_FIELD(Bool, bool);
							READ_SCRIPT_FIELD(Char, char);
							READ_SCRIPT_FIELD(Byte, int8_t);
							READ_SCRIPT_FIELD(Short, int16_t);
							READ_SCRIPT_FIELD(Int, int32_t);
							READ_SCRIPT_FIELD(Long, int64_t);
							READ_SCRIPT_FIELD(UByte, uint8_t);
							READ_SCRIPT_FIELD(UShort, uint16_t);
							READ_SCRIPT_FIELD(UInt, uint32_t);
							READ_SCRIPT_FIELD(ULong, uint64_t);
							READ_SCRIPT_FIELD(Vector2, glm::vec2);
							READ_SCRIPT_FIELD(Vector3, glm::vec3);
							READ_SCRIPT_FIELD(Vector4, glm::vec4);
							READ_SCRIPT_FIELD(Entity, UUID);
						}
					}
				}
			}

			#undef READ_SCRIPT_FIELD
		}

	} // namespace

	// Deserialize BIN
	namespace
	{
		template<>
		void ComponentDeserializeBinIfExist<TagComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (p_Current != "TagComponent")
				return;

			auto& comp = p_Entity.GetComponent<TagComponent>();
			comp.Tag = Utils::ReadString(p_In);
		}

		template<>
		void ComponentDeserializeBinIfExist<HierarchyComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (p_Current != "HierarchyComponent")
				return;

			UUID parent, first, prev, next;
			p_In.read(reinterpret_cast<char*>(&parent), sizeof(UUID));
			p_In.read(reinterpret_cast<char*>(&first), sizeof(UUID));
			p_In.read(reinterpret_cast<char*>(&prev), sizeof(UUID));
			p_In.read(reinterpret_cast<char*>(&next), sizeof(UUID));

			auto& comp = p_Entity.AddOrReplaceComponent<HierarchyComponent>();
			auto scene = p_Entity.GetScene();
			comp.Parent = parent != 0ul ? scene->GetEntityByUUID(parent).GetHandle() : entt::null;
			comp.First = first != 0ul ? scene->GetEntityByUUID(first).GetHandle() : entt::null;
			comp.Prev = prev != 0ul ? scene->GetEntityByUUID(prev).GetHandle() : entt::null;
			comp.Next = next != 0ul ? scene->GetEntityByUUID(next).GetHandle() : entt::null;

			p_In.read(reinterpret_cast<char*>(&comp.ChildCount), sizeof(comp.ChildCount));
		}

		template<>
		void ComponentDeserializeBinIfExist<TransformComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (p_Current != "TransformComponent")
				return;

			auto& comp = p_Entity.AddOrReplaceComponent<TransformComponent>();
			glm::vec3 translation, rotation, scale;
			p_In.read(reinterpret_cast<char*>(&translation), sizeof(translation));
			p_In.read(reinterpret_cast<char*>(&rotation), sizeof(rotation));
			p_In.read(reinterpret_cast<char*>(&scale), sizeof(scale));

			comp.SetLocalTranslation(translation);
			comp.SetLocalRotation(rotation);
			comp.SetLocalScale(scale);
		}

		template<>
		void ComponentDeserializeBinIfExist<CameraComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (p_Current != "CameraComponent")
				return;

			auto& comp = p_Entity.AddOrReplaceComponent<CameraComponent>();

			p_In.read(reinterpret_cast<char*>(&comp.Primary), sizeof(comp.Primary));
			p_In.read(reinterpret_cast<char*>(&comp.ClearColor), sizeof(comp.ClearColor));

			uint32_t width, height;
			p_In.read(reinterpret_cast<char*>(&width), sizeof(width));
			p_In.read(reinterpret_cast<char*>(&height), sizeof(height));

			bool isOrthographic, isAspectRatioFixed;
			p_In.read(reinterpret_cast<char*>(&isOrthographic), sizeof(isOrthographic));
			p_In.read(reinterpret_cast<char*>(&isAspectRatioFixed), sizeof(isAspectRatioFixed));

			float fov, farZ, nearZ, scale, zoom;
			p_In.read(reinterpret_cast<char*>(&fov), sizeof(fov));
			p_In.read(reinterpret_cast<char*>(&farZ), sizeof(farZ));
			p_In.read(reinterpret_cast<char*>(&nearZ), sizeof(nearZ));
			p_In.read(reinterpret_cast<char*>(&scale), sizeof(scale));
			p_In.read(reinterpret_cast<char*>(&zoom), sizeof(zoom));

			comp.Camera.SetViewportSize(width, height);
			comp.Camera.SetIsOrthographic(isOrthographic);
			comp.Camera.SetFixAspectRatio(isAspectRatioFixed);
			comp.Camera.SetFOV(fov);
			comp.Camera.SetFar(farZ);
			comp.Camera.SetNear(nearZ);
			comp.Camera.SetScale(scale);
			comp.Camera.SetZoom(zoom);
		}

		template<>
		void ComponentDeserializeBinIfExist<SpriteComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (p_Current != "SpriteComponent")
				return;

			auto& comp = p_Entity.AddOrReplaceComponent<SpriteComponent>();

			p_In.read(reinterpret_cast<char*>(&comp.Type), sizeof(comp.Type));
			p_In.read(reinterpret_cast<char*>(&comp.Color), sizeof(comp.Color));
			p_In.read(reinterpret_cast<char*>(&comp.Texture), sizeof(comp.Texture));
			p_In.read(reinterpret_cast<char*>(&comp.Thickness), sizeof(comp.Thickness));
			p_In.read(reinterpret_cast<char*>(&comp.Fade), sizeof(comp.Fade));
			p_In.read(reinterpret_cast<char*>(&comp.Size), sizeof(comp.Size));
			p_In.read(reinterpret_cast<char*>(&comp.BySize), sizeof(comp.BySize));
			p_In.read(reinterpret_cast<char*>(&comp.Offset), sizeof(comp.Offset));
			p_In.read(reinterpret_cast<char*>(&comp.Scale), sizeof(comp.Scale));
		}

		template<>
		void ComponentDeserializeBinIfExist<LineRendererComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (p_Current != "LineRendererComponent")
				return;

			auto& comp = p_Entity.AddOrReplaceComponent<LineRendererComponent>();

			p_In.read(reinterpret_cast<char*>(&comp.Color), sizeof(comp.Color));
			p_In.read(reinterpret_cast<char*>(&comp.Width), sizeof(comp.Width));
			p_In.read(reinterpret_cast<char*>(&comp.Primitive), sizeof(comp.Primitive));
			p_In.read(reinterpret_cast<char*>(&comp.Start), sizeof(comp.Start));
			p_In.read(reinterpret_cast<char*>(&comp.End), sizeof(comp.End));
		}

		template<>
		void ComponentDeserializeBinIfExist<TextRendererComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (p_Current != "TextRendererComponent")
				return;

			auto& comp = p_Entity.AddOrReplaceComponent<TextRendererComponent>();

			comp.String = Utils::ReadString(p_In);
			p_In.read(reinterpret_cast<char*>(&comp.Font), sizeof(comp.Font));
			p_In.read(reinterpret_cast<char*>(&comp.Color), sizeof(comp.Color));
			p_In.read(reinterpret_cast<char*>(&comp.BgColor), sizeof(comp.BgColor));
			p_In.read(reinterpret_cast<char*>(&comp.CharBgColor), sizeof(comp.CharBgColor));
			p_In.read(reinterpret_cast<char*>(&comp.DrawBg), sizeof(comp.DrawBg));
			p_In.read(reinterpret_cast<char*>(&comp.Kerning), sizeof(comp.Kerning));
			p_In.read(reinterpret_cast<char*>(&comp.LineSpacing), sizeof(comp.LineSpacing));
		}

		template<>
		void ComponentDeserializeBinIfExist<Rigidbody2DComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (p_Current != "Rigidbody2DComponent")
				return;

			auto& comp = p_Entity.AddOrReplaceComponent<Rigidbody2DComponent>();

			p_In.read(reinterpret_cast<char*>(&comp.Type), sizeof(comp.Type));
			p_In.read(reinterpret_cast<char*>(&comp.FixedRotation), sizeof(comp.FixedRotation));
			p_In.read(reinterpret_cast<char*>(&comp.GravityScale), sizeof(comp.GravityScale));
		}

		template<>
		void ComponentDeserializeBinIfExist<BoxCollider2DComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (p_Current != "BoxCollider2DComponent")
				return;

			auto& comp = p_Entity.AddOrReplaceComponent<BoxCollider2DComponent>();

			p_In.read(reinterpret_cast<char*>(&comp.IsTrigger), sizeof(comp.IsTrigger));
			p_In.read(reinterpret_cast<char*>(&comp.Offset), sizeof(comp.Offset));
			p_In.read(reinterpret_cast<char*>(&comp.Size), sizeof(comp.Size));
			p_In.read(reinterpret_cast<char*>(&comp.Density), sizeof(comp.Density));
			p_In.read(reinterpret_cast<char*>(&comp.Friction), sizeof(comp.Friction));
			p_In.read(reinterpret_cast<char*>(&comp.Restitution), sizeof(comp.Restitution));
			p_In.read(reinterpret_cast<char*>(&comp.RestitutionThreshold), sizeof(comp.RestitutionThreshold));
		}

		template<>
		void ComponentDeserializeBinIfExist<CircleCollider2DComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (p_Current != "CircleCollider2DComponent")
				return;

			auto& comp = p_Entity.AddOrReplaceComponent<CircleCollider2DComponent>();

			p_In.read(reinterpret_cast<char*>(&comp.IsTrigger), sizeof(comp.IsTrigger));
			p_In.read(reinterpret_cast<char*>(&comp.Offset), sizeof(comp.Offset));
			p_In.read(reinterpret_cast<char*>(&comp.Radius), sizeof(comp.Radius));
			p_In.read(reinterpret_cast<char*>(&comp.Density), sizeof(comp.Density));
			p_In.read(reinterpret_cast<char*>(&comp.Friction), sizeof(comp.Friction));
			p_In.read(reinterpret_cast<char*>(&comp.Restitution), sizeof(comp.Restitution));
			p_In.read(reinterpret_cast<char*>(&comp.RestitutionThreshold), sizeof(comp.RestitutionThreshold));
		}

		template<>
		void ComponentDeserializeBinIfExist<ScriptComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			#define READ_SCRIPT_FIELD(FieldType, Type)             \
				case ScriptFieldType::FieldType:                   \
				{                                                  \
					Type data;                                      \
					p_In.read(reinterpret_cast<char*>(&data), sizeof(Type)); \
					fieldInstance.SetValue(data);                  \
					break;                                         \
				}

			if (p_Current != "ScriptComponent")
				return;

			auto& comp = p_Entity.AddOrReplaceComponent<ScriptComponent>();
			comp.FullClassName = Utils::ReadString(p_In);

			auto entityClass = ScriptEngine::GetEntityClass(comp.FullClassName);
			const auto& fields = entityClass->GetFields();

			if (fields.size() > 0)
			{
				auto& entityFields = ScriptEngine::GetScriptFieldMap(p_Entity);

				int size = 0;
				p_In.read(reinterpret_cast<char*>(&size), sizeof(size));

				for (int i = 0; i < size; i++)
				{
					std::string name = Utils::ReadString(p_In);

					ScriptFieldType type;
					p_In.read(reinterpret_cast<char*>(&type), sizeof(type));

					ScriptFieldInstance& fieldInstance = entityFields[name];
					if (fields.find(name) == fields.end())
					{
						KTN_CORE_WARN("Script field '{}' not found in class '{}'. Skipping...", name, comp.FullClassName);
						continue;
					}
					fieldInstance.Field = fields.at(name);

					switch (type)
					{
						READ_SCRIPT_FIELD(Float, float);
						READ_SCRIPT_FIELD(Double, double);
						READ_SCRIPT_FIELD(Bool, bool);
						READ_SCRIPT_FIELD(Char, char);
						READ_SCRIPT_FIELD(Byte, int8_t);
						READ_SCRIPT_FIELD(Short, int16_t);
						READ_SCRIPT_FIELD(Int, int32_t);
						READ_SCRIPT_FIELD(Long, int64_t);
						READ_SCRIPT_FIELD(UByte, uint8_t);
						READ_SCRIPT_FIELD(UShort, uint16_t);
						READ_SCRIPT_FIELD(UInt, uint32_t);
						READ_SCRIPT_FIELD(ULong, uint64_t);
						READ_SCRIPT_FIELD(Vector2, glm::vec2);
						READ_SCRIPT_FIELD(Vector3, glm::vec3);
						READ_SCRIPT_FIELD(Vector4, glm::vec4);
						READ_SCRIPT_FIELD(Entity, UUID);
					}
				}
			}
			#undef READ_SCRIPT_FIELD
		}

	} // namespace

} // namespace KTN
