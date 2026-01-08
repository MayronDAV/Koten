#include "ktnpch.h"
#include "EntitySerializer.h"
#include "Koten/Script/ScriptEngine.h"
#include "Koten/Physics/PhysicsMaterial2D.h"
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

	void EntitySerializer::Serialize(YAML::Emitter* p_Out, const Entity& p_Entt)
	{
		KTN_PROFILE_FUNCTION();

		ComponentSerialize<ALL_COMPONENTS>(*p_Out, p_Entt.GetScene()->GetRegistry(), p_Entt);
	}

	bool EntitySerializer::Deserialize(YAML::Node* p_Node, Entity& p_Entt)
	{
		KTN_PROFILE_FUNCTION();

		ComponentDeserialize<ALL_COMPONENTS>(*p_Node, p_Entt.GetScene()->GetRegistry(), p_Entt);

		return true;
	}

	void EntitySerializer::SerializeBin(std::ofstream& p_Out, const Entity& p_Entt)
	{
		KTN_PROFILE_FUNCTION();

		ComponentSerializeBin<ALL_COMPONENTS>(p_Out, p_Entt.GetScene()->GetRegistry(), p_Entt);
	}

	bool EntitySerializer::DeserializeBin(std::ifstream& p_In, Entity& p_Entt)
	{
		KTN_PROFILE_FUNCTION();

		ComponentDeserializeBin<ALL_COMPONENTS>(p_In, p_Entt.GetScene()->GetRegistry(), p_Entt);

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
		void ComponentSerializeIfExist<CharacterBody2DComponent>(YAML::Emitter& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<CharacterBody2DComponent>())
				return;

			p_Out << YAML::Key << "CharacterBody2DComponent";
			p_Out << YAML::BeginMap;

			auto& comp = p_Entity.GetComponent<CharacterBody2DComponent>();
			ADD_KEY_VALUE("IsTrigger", comp.IsTrigger);
			ADD_KEY_VALUE("Mass", comp.Mass);
			ADD_KEY_VALUE("PhysicsMaterial2D", comp.PhysicsMaterial2D);
			ADD_KEY_VALUE("SlideOnCeiling", comp.SlideOnCeiling);
			ADD_KEY_VALUE("FloorMaxAngle", comp.FloorMaxAngle);
			ADD_KEY_VALUE("FloorSnapLength", comp.FloorSnapLength);
			ADD_KEY_VALUE("WallMinSlideAngle", comp.WallMinSlideAngle);
			ADD_KEY_VALUE("UpDirection", comp.UpDirection);
			ADD_KEY_VALUE("FloorNormal", comp.FloorNormal);

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
			ADD_KEY_VALUE("IsTrigger", comp.IsTrigger);
			ADD_KEY_VALUE("Mass", comp.Mass);
			ADD_KEY_VALUE("PhysicsMaterial2D", comp.PhysicsMaterial2D);
			ADD_KEY_VALUE("LinearDamping", comp.LinearDamping);
			ADD_KEY_VALUE("AngularDamping", comp.AngularDamping);
			ADD_KEY_VALUE("GravityScale", comp.GravityScale);
			ADD_KEY_VALUE("FixedRotation", comp.FixedRotation);
			ADD_KEY_VALUE("Sleeping", comp.Sleeping);
			ADD_KEY_VALUE("CanSleep", comp.CanSleep);

			p_Out << YAML::EndMap;
		}

		template<>
		void ComponentSerializeIfExist<StaticBody2DComponent>(YAML::Emitter& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<StaticBody2DComponent>())
				return;

			p_Out << YAML::Key << "StaticBody2DComponent";
			p_Out << YAML::BeginMap;

			auto& comp = p_Entity.GetComponent<StaticBody2DComponent>();
			ADD_KEY_VALUE("IsTrigger", comp.IsTrigger);
			ADD_KEY_VALUE("Mass", comp.Mass);
			ADD_KEY_VALUE("PhysicsMaterial2D", comp.PhysicsMaterial2D);

			p_Out << YAML::EndMap;
		}

		template<>
		void ComponentSerializeIfExist<BodyShape2DComponent>(YAML::Emitter& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<BodyShape2DComponent>())
				return;

			p_Out << YAML::Key << "BodyShape2DComponent";
			p_Out << YAML::BeginMap;

			auto& comp = p_Entity.GetComponent<BodyShape2DComponent>();

			ADD_KEY_VALUE("Shape", (int)comp.Shape);
			ADD_KEY_VALUE("Offset", comp.Offset);
			ADD_KEY_VALUE("Size", comp.Size);

			p_Out << YAML::EndMap;
		}

		template<>
		void ComponentSerializeIfExist<ScriptComponent>(YAML::Emitter& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			#define WRITE_SCRIPT_FIELD(FieldType, Type)    \
				case ScriptFieldType::FieldType:           \
					p_Out << scriptField.GetValue<Type>(); \
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

		template<>
		void ComponentSerializeIfExist<PrefabComponent>(YAML::Emitter& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<PrefabComponent>())
				return;

			p_Out << YAML::Key << "PrefabComponent";
			p_Out << YAML::BeginMap;

			auto& path = p_Entity.GetComponent<PrefabComponent>().Path;
			auto rel = FileSystem::GetRelative(path, Project::GetAssetDirectory().string());
			ADD_KEY_VALUE("Path", rel);

			p_Out << YAML::EndMap;
		}

		template<>
		void ComponentSerializeIfExist<RuntimeComponent>(YAML::Emitter& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<RuntimeComponent>())
				return;

			p_Out << YAML::Key << "RuntimeComponent";
			p_Out << YAML::BeginMap;

			auto& comp = p_Entity.GetComponent<RuntimeComponent>();
			ADD_KEY_VALUE("Enabled", comp.Enabled);
			ADD_KEY_VALUE("Active", comp.Active);

			p_Out << YAML::EndMap;
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
		void ComponentSerializeBinIfExist<CharacterBody2DComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<CharacterBody2DComponent>())
				return;

			Utils::WriteString(p_Out, "CharacterBody2DComponent");

			auto& comp = p_Entity.GetComponent<CharacterBody2DComponent>();

			p_Out.write(reinterpret_cast<const char*>(&comp.IsTrigger), sizeof(comp.IsTrigger));
			p_Out.write(reinterpret_cast<const char*>(&comp.Mass), sizeof(comp.Mass));
			p_Out.write(reinterpret_cast<const char*>(&comp.PhysicsMaterial2D), sizeof(comp.PhysicsMaterial2D));
			p_Out.write(reinterpret_cast<const char*>(&comp.SlideOnCeiling), sizeof(comp.SlideOnCeiling));
			p_Out.write(reinterpret_cast<const char*>(&comp.FloorMaxAngle), sizeof(comp.FloorMaxAngle));
			p_Out.write(reinterpret_cast<const char*>(&comp.FloorSnapLength), sizeof(comp.FloorSnapLength));
			p_Out.write(reinterpret_cast<const char*>(&comp.WallMinSlideAngle), sizeof(comp.WallMinSlideAngle));
			p_Out.write(reinterpret_cast<const char*>(&comp.UpDirection), sizeof(comp.UpDirection));
			p_Out.write(reinterpret_cast<const char*>(&comp.FloorNormal), sizeof(comp.FloorNormal));
		}

		template<>
		void ComponentSerializeBinIfExist<Rigidbody2DComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<Rigidbody2DComponent>())
				return;

			Utils::WriteString(p_Out, "Rigidbody2DComponent");

			auto& comp = p_Entity.GetComponent<Rigidbody2DComponent>();

			p_Out.write(reinterpret_cast<const char*>(&comp.IsTrigger), sizeof(comp.IsTrigger));
			p_Out.write(reinterpret_cast<const char*>(&comp.Mass), sizeof(comp.Mass));
			p_Out.write(reinterpret_cast<const char*>(&comp.PhysicsMaterial2D), sizeof(comp.PhysicsMaterial2D));
			p_Out.write(reinterpret_cast<const char*>(&comp.LinearDamping), sizeof(comp.LinearDamping));
			p_Out.write(reinterpret_cast<const char*>(&comp.AngularDamping), sizeof(comp.AngularDamping));
			p_Out.write(reinterpret_cast<const char*>(&comp.GravityScale), sizeof(comp.GravityScale));
			p_Out.write(reinterpret_cast<const char*>(&comp.FixedRotation), sizeof(comp.FixedRotation));
			p_Out.write(reinterpret_cast<const char*>(&comp.Sleeping), sizeof(comp.Sleeping));
			p_Out.write(reinterpret_cast<const char*>(&comp.CanSleep), sizeof(comp.CanSleep));
		}

		template<>
		void ComponentSerializeBinIfExist<StaticBody2DComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<StaticBody2DComponent>())
				return;

			Utils::WriteString(p_Out, "StaticBody2DComponent");

			auto& comp = p_Entity.GetComponent<StaticBody2DComponent>();

			p_Out.write(reinterpret_cast<const char*>(&comp.IsTrigger), sizeof(comp.IsTrigger));
			p_Out.write(reinterpret_cast<const char*>(&comp.Mass), sizeof(comp.Mass));
			p_Out.write(reinterpret_cast<const char*>(&comp.PhysicsMaterial2D), sizeof(comp.PhysicsMaterial2D));
		}

		template<>
		void ComponentSerializeBinIfExist<BodyShape2DComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<BodyShape2DComponent>())
				return;

			Utils::WriteString(p_Out, "BodyShape2DComponent");

			auto& comp = p_Entity.GetComponent<BodyShape2DComponent>();

			p_Out.write(reinterpret_cast<const char*>(&comp.Shape), sizeof(comp.Shape));
			p_Out.write(reinterpret_cast<const char*>(&comp.Offset), sizeof(comp.Offset));
			p_Out.write(reinterpret_cast<const char*>(&comp.Size), sizeof(comp.Size));
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

		template<>
		void ComponentSerializeBinIfExist<PrefabComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<PrefabComponent>())
				return;

			Utils::WriteString(p_Out, "PrefabComponent");

			auto rel = FileSystem::GetRelative(p_Entity.GetComponent<PrefabComponent>().Path, Project::GetAssetDirectory().string());
			Utils::WriteString(p_Out, rel);
		}

		template<>
		void ComponentSerializeBinIfExist<RuntimeComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<RuntimeComponent>())
				return;

			Utils::WriteString(p_Out, "RuntimeComponent");

			auto& comp = p_Entity.GetComponent<RuntimeComponent>();
			p_Out.write(reinterpret_cast<const char*>(&comp.Enabled), sizeof(comp.Enabled));
			p_Out.write(reinterpret_cast<const char*>(&comp.Active), sizeof(comp.Active));
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

				p_Entity.AddOrReplaceComponent<TransformComponent>(translation, scale, rotation);
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

				auto& comp = p_Entity.AddOrReplaceComponent<CameraComponent>();
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
				auto& comp = p_Entity.AddOrReplaceComponent<SpriteComponent>();
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
		void ComponentDeserializeIfExist<CharacterBody2DComponent>(YAML::Node& p_Data, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			auto data = p_Data["CharacterBody2DComponent"];
			if (!data) return;

			auto& comp = p_Entity.AddOrReplaceComponent<CharacterBody2DComponent>();

			comp.IsTrigger = data["IsTrigger"].as<bool>();
			comp.Mass = data["Mass"].as<float>();
			comp.PhysicsMaterial2D = data["PhysicsMaterial2D"] ? data["PhysicsMaterial2D"].as<AssetHandle>() : PhysicsMaterial2D::GetDefault();
			comp.SlideOnCeiling = data["SlideOnCeiling"].as<bool>();
			comp.FloorMaxAngle = data["FloorMaxAngle"].as<float>();
			comp.FloorSnapLength = data["FloorSnapLength"].as<float>();
			comp.WallMinSlideAngle = data["WallMinSlideAngle"].as<float>();
			comp.UpDirection = data["UpDirection"].as<glm::vec2>();
			comp.FloorNormal = data["FloorNormal"].as<glm::vec2>();
		}

		template<>
		void ComponentDeserializeIfExist<Rigidbody2DComponent>(YAML::Node& p_Data, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			auto data = p_Data["Rigidbody2DComponent"];
			if (!data) return;

			auto& comp = p_Entity.AddOrReplaceComponent<Rigidbody2DComponent>();

			comp.IsTrigger = data["IsTrigger"].as<bool>();
			comp.Mass = data["Mass"].as<float>();
			comp.PhysicsMaterial2D = data["PhysicsMaterial2D"] ? data["PhysicsMaterial2D"].as<AssetHandle>() : PhysicsMaterial2D::GetDefault();
			comp.LinearDamping = data["LinearDamping"].as<float>();
			comp.AngularDamping = data["AngularDamping"].as<float>();
			comp.GravityScale = data["GravityScale"].as<float>();
			comp.FixedRotation = data["FixedRotation"].as<bool>();
			comp.Sleeping = data["Sleeping"].as<bool>();
			comp.CanSleep = data["CanSleep"].as<bool>();
		}

		template<>
		void ComponentDeserializeIfExist<StaticBody2DComponent>(YAML::Node& p_Data, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			auto data = p_Data["StaticBody2DComponent"];
			if (!data) return;

			auto& comp = p_Entity.AddOrReplaceComponent<StaticBody2DComponent>();

			comp.IsTrigger = data["IsTrigger"].as<bool>();
			comp.Mass = data["Mass"].as<float>();
			comp.PhysicsMaterial2D = data["PhysicsMaterial2D"] ? data["PhysicsMaterial2D"].as<AssetHandle>() : PhysicsMaterial2D::GetDefault();
		}

		template<>
		void ComponentDeserializeIfExist<BodyShape2DComponent>(YAML::Node& p_Data, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			auto data = p_Data["BodyShape2DComponent"];
			if (!data) return;

			auto& comp = p_Entity.AddOrReplaceComponent<BodyShape2DComponent>();

			comp.Shape = (Shape2D)data["Shape"].as<int>();
			comp.Offset = data["Offset"].as<glm::vec2>();
			comp.Size = data["Size"].as<glm::vec2>();
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

		template<>
		void ComponentDeserializeIfExist<PrefabComponent>(YAML::Node& p_Data, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			auto data = p_Data["PrefabComponent"];
			if (!data) return;

			auto& comp = p_Entity.AddOrReplaceComponent<PrefabComponent>();
			comp.Path = data["Path"].as<std::string>();
			comp.Path = Project::GetAssetFileSystemPath(comp.Path).string();
		}

		template<>
		void ComponentDeserializeIfExist<RuntimeComponent>(YAML::Node& p_Data, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			auto data = p_Data["RuntimeComponent"];
			if (!data) return;

			auto& comp = p_Entity.AddOrReplaceComponent<RuntimeComponent>();
			comp.Enabled = data["Enabled"].as<bool>();
			comp.Active = data["Active"].as<bool>();
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
		void ComponentDeserializeBinIfExist<CharacterBody2DComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (p_Current != "CharacterBody2DComponent")
				return;

			auto& comp = p_Entity.AddOrReplaceComponent<CharacterBody2DComponent>();

			p_In.read(reinterpret_cast<char*>(&comp.IsTrigger), sizeof(comp.IsTrigger));
			p_In.read(reinterpret_cast<char*>(&comp.Mass), sizeof(comp.Mass));
			p_In.read(reinterpret_cast<char*>(&comp.PhysicsMaterial2D), sizeof(comp.PhysicsMaterial2D));
			p_In.read(reinterpret_cast<char*>(&comp.SlideOnCeiling), sizeof(comp.SlideOnCeiling));
			p_In.read(reinterpret_cast<char*>(&comp.FloorMaxAngle), sizeof(comp.FloorMaxAngle));
			p_In.read(reinterpret_cast<char*>(&comp.FloorSnapLength), sizeof(comp.FloorSnapLength));
			p_In.read(reinterpret_cast<char*>(&comp.WallMinSlideAngle), sizeof(comp.WallMinSlideAngle));
			p_In.read(reinterpret_cast<char*>(&comp.UpDirection), sizeof(comp.UpDirection));
			p_In.read(reinterpret_cast<char*>(&comp.FloorNormal), sizeof(comp.FloorNormal));
		}

		template<>
		void ComponentDeserializeBinIfExist<Rigidbody2DComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (p_Current != "Rigidbody2DComponent")
				return;

			auto& comp = p_Entity.AddOrReplaceComponent<Rigidbody2DComponent>();

			p_In.read(reinterpret_cast<char*>(&comp.IsTrigger), sizeof(comp.IsTrigger));
			p_In.read(reinterpret_cast<char*>(&comp.Mass), sizeof(comp.Mass));
			p_In.read(reinterpret_cast<char*>(&comp.PhysicsMaterial2D), sizeof(comp.PhysicsMaterial2D));
			p_In.read(reinterpret_cast<char*>(&comp.LinearDamping), sizeof(comp.LinearDamping));
			p_In.read(reinterpret_cast<char*>(&comp.AngularDamping), sizeof(comp.AngularDamping));
			p_In.read(reinterpret_cast<char*>(&comp.GravityScale), sizeof(comp.GravityScale));
			p_In.read(reinterpret_cast<char*>(&comp.FixedRotation), sizeof(comp.FixedRotation));
			p_In.read(reinterpret_cast<char*>(&comp.Sleeping), sizeof(comp.Sleeping));
			p_In.read(reinterpret_cast<char*>(&comp.CanSleep), sizeof(comp.CanSleep));
		}

		template<>
		void ComponentDeserializeBinIfExist<StaticBody2DComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (p_Current != "StaticBody2DComponent")
				return;

			auto& comp = p_Entity.AddOrReplaceComponent<StaticBody2DComponent>();

			p_In.read(reinterpret_cast<char*>(&comp.IsTrigger), sizeof(comp.IsTrigger));
			p_In.read(reinterpret_cast<char*>(&comp.Mass), sizeof(comp.Mass));
			p_In.read(reinterpret_cast<char*>(&comp.PhysicsMaterial2D), sizeof(comp.PhysicsMaterial2D));
		}

		template<>
		void ComponentDeserializeBinIfExist<BodyShape2DComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (p_Current != "BodyShape2DComponent")
				return;

			auto& comp = p_Entity.AddOrReplaceComponent<BodyShape2DComponent>();

			p_In.read(reinterpret_cast<char*>(&comp.Shape), sizeof(comp.Shape));
			p_In.read(reinterpret_cast<char*>(&comp.Offset), sizeof(comp.Offset));
			p_In.read(reinterpret_cast<char*>(&comp.Size), sizeof(comp.Size));
		}

		template<>
		void ComponentDeserializeBinIfExist<ScriptComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			#define READ_SCRIPT_FIELD(FieldType, Type)                       \
				case ScriptFieldType::FieldType:                             \
				{                                                            \
					Type data;                                               \
					p_In.read(reinterpret_cast<char*>(&data), sizeof(Type)); \
					fieldInstance.SetValue(data);                            \
					break;                                                   \
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

		template<>
		void ComponentDeserializeBinIfExist<PrefabComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (p_Current != "PrefabComponent")
				return;

			auto& comp = p_Entity.AddOrReplaceComponent<PrefabComponent>();
			comp.Path = Utils::ReadString(p_In);
			comp.Path = Project::GetAssetFileSystemPath(comp.Path).string();
		}

		template<>
		void ComponentDeserializeBinIfExist<RuntimeComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (p_Current != "RuntimeComponent")
				return;

			auto& comp = p_Entity.GetComponent<RuntimeComponent>();
			p_In.read(reinterpret_cast<char*>(&comp.Enabled), sizeof(comp.Enabled));
			p_In.read(reinterpret_cast<char*>(&comp.Active), sizeof(comp.Active));
		}

	} // namespace
}
