#include "ktnpch.h"
#include "SceneSerializer.h"
#include "Entity.h"
#include "Koten/Graphics/TextureImporter.h"

// lib
#include <yaml-cpp/yaml.h>


namespace YAML
{
	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& p_Rhs)
		{
			Node node;
			node.push_back(p_Rhs.x);
			node.push_back(p_Rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& p_Rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			p_Rhs.x = node[0].as<float>();
			p_Rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& p_Rhs)
		{
			Node node;
			node.push_back(p_Rhs.x);
			node.push_back(p_Rhs.y);
			node.push_back(p_Rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& p_Rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			p_Rhs.x = node[0].as<float>();
			p_Rhs.y = node[1].as<float>();
			p_Rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& p_Rhs)
		{
			Node node;
			node.push_back(p_Rhs.x);
			node.push_back(p_Rhs.y);
			node.push_back(p_Rhs.z);
			node.push_back(p_Rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& p_Rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			p_Rhs.x = node[0].as<float>();
			p_Rhs.y = node[1].as<float>();
			p_Rhs.z = node[2].as<float>();
			p_Rhs.w = node[3].as<float>();
			return true;
		}
	};

	Emitter& operator <<(Emitter& p_Out, const glm::vec2& p_Value)
	{
		p_Out << Flow;
		p_Out << BeginSeq << p_Value.x << p_Value.y << EndSeq;
		return p_Out;
	}

	Emitter& operator <<(Emitter& p_Out, const glm::vec3& p_Value)
	{
		p_Out << Flow;
		p_Out << BeginSeq << p_Value.x << p_Value.y << p_Value.z << EndSeq;
		return p_Out;
	}

	Emitter& operator <<(Emitter& p_Out, const glm::vec4& p_Value)
	{
		p_Out << Flow;
		p_Out << BeginSeq << p_Value.x << p_Value.y << p_Value.z << p_Value.w << EndSeq;
		return p_Out;
	}

} // namespace YAML


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
		out << YAML::Key << "Scene" << YAML::Value << "435253463464"; // TODO: UUID
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

		auto sceneID = data["Scene"].as<std::string>(); // TODO
		KTN_CORE_INFO("Deserializing scene: (filename: '{}', id: '{}')", FileSystem::GetName(p_Filepath), sceneID);

		auto entts = data["Entities"];
		if (entts)
		{
			for (auto entt : entts)
			{
				auto enttID = entt["Entity"].as<uint64_t>();
				Entity deserializedEntt = m_Scene->CreateEntity((UUID)enttID, "Entity");

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

			ADD_KEY_VALUE("Parent", (uint32_t)comp.Parent);
			ADD_KEY_VALUE("First", (uint32_t)comp.First);
			ADD_KEY_VALUE("Prev", (uint32_t)comp.Prev);
			ADD_KEY_VALUE("Next", (uint32_t)comp.Next);
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
			ADD_KEY_VALUE("Path", comp.Path);
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
			// TODO: make this when we have UUID
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
				auto type = (RenderType2D)spriteComp["Type"].as<int>();
				auto color = spriteComp["Color"].as<glm::vec4>();
				auto path = spriteComp["Path"].as<std::string>();
				auto thickness = spriteComp["Thickness"].as<float>();
				auto fade = spriteComp["Fade"].as<float>();
				auto size = spriteComp["Size"].as<glm::vec2>();
				auto bySize = spriteComp["BySize"].as<bool>();
				auto offset = spriteComp["Offset"].as<glm::vec2>();
				auto scale = spriteComp["Scale"].as<glm::vec2>();

				auto& comp = p_Entity.AddComponent<SpriteComponent>(color);	
				comp.Type = type;
				comp.Path = path;
				comp.Thickness = thickness;
				comp.Fade = fade;
				comp.Size = size;
				comp.BySize = bySize;
				comp.Offset = offset;
				comp.Scale = scale;

				// TODO: Change this when we have projects.
				if (path != "")
				{
					comp.Texture = TextureImporter::LoadTexture2D(path);
				}
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

	} // namespace

} // namespace KTN
