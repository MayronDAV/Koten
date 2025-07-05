#include "ktnpch.h"
#include "SceneSerializer.h"
#include "Entity.h"
#include "Koten/Asset/TextureImporter.h"
#include "Koten/Script/ScriptEngine.h"
#include "Koten/Project/Project.h"

// lib
#include <yaml-cpp/yaml.h>




namespace KTN
{
	namespace
	{
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

	// Serialize
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
			auto bodyId = comp.Body;
			ADD_KEY_VALUE("Index", bodyId.Index);
			ADD_KEY_VALUE("World", bodyId.World);
			ADD_KEY_VALUE("Generation", bodyId.Generation);
			ADD_KEY_VALUE("Type", (int)comp.Type);
			ADD_KEY_VALUE("FixedRotation", comp.FixedRotation);

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
						WRITE_SCRIPT_FIELD(Float,   float);
						WRITE_SCRIPT_FIELD(Double,  double);
						WRITE_SCRIPT_FIELD(Bool,    bool);
						WRITE_SCRIPT_FIELD(Char,    char);
						WRITE_SCRIPT_FIELD(Byte,    int8_t);
						WRITE_SCRIPT_FIELD(Short,   int16_t);
						WRITE_SCRIPT_FIELD(Int,     int32_t);
						WRITE_SCRIPT_FIELD(Long,    int64_t);
						WRITE_SCRIPT_FIELD(UByte,   uint8_t);
						WRITE_SCRIPT_FIELD(UShort,  uint16_t);
						WRITE_SCRIPT_FIELD(UInt,    uint32_t);
						WRITE_SCRIPT_FIELD(ULong,   uint64_t);
						WRITE_SCRIPT_FIELD(Vector2, glm::vec2);
						WRITE_SCRIPT_FIELD(Vector3, glm::vec3);
						WRITE_SCRIPT_FIELD(Vector4, glm::vec4);
						WRITE_SCRIPT_FIELD(Entity,  UUID);
					}
					p_Out << YAML::EndMap; // ScriptFields
				}
				p_Out << YAML::EndSeq;
			}

			p_Out << YAML::EndMap;

			#undef WRITE_SCRIPT_FIELD
		}

	} // namespace

	// Deserialize
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
			comp.Body.Index = rigidComp["Index"].as<int32_t>();
			comp.Body.World = rigidComp["World"].as<uint16_t>();
			comp.Body.Generation = rigidComp["Generation"].as<uint16_t>();
		}

		template<>
		void ComponentDeserializeIfExist<BoxCollider2DComponent>(YAML::Node& p_Data, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			auto boxComp = p_Data["BoxCollider2DComponent"];
			if (!boxComp) return;

			auto& comp = p_Entity.AddOrReplaceComponent<BoxCollider2DComponent>();

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

} // namespace KTN
