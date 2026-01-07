#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Math/Transform.h"
#include "Koten/Graphics/Texture.h"
#include "Koten/Graphics/Camera.h"
#include "Koten/Core/UUID.h"
#include "Koten/Graphics/DFFont.h"
#include "Koten/Asset/Asset.h"
#include "Koten/Physics/PhysicsMaterial2D.h"

// std
#include <string>

// lib
#include <entt/entt.hpp>



namespace KTN
{
	using SceneCamera = Camera;
	using TransformComponent = Math::Transform;

	struct IDComponent
	{
		UUID ID;

		inline operator UUID() const { return ID; }

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(const UUID& p_UUID)
			: ID(p_UUID) {}
	};

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& p_Tag)
			: Tag(p_Tag) {}
	};

	struct SpriteComponent
	{
		RenderType2D Type		= RenderType2D::Quad;
		AssetHandle Texture		= 0;
		glm::vec4 Color			= { 1.0f, 1.0f, 1.0f, 1.0f };
		float Thickness			= 1.0f;
		float Fade				= 0.005f;

		glm::vec2 Size			= { 0.0f, 0.0f };
		// [true] if you want to pass the tile coord as a multiplier of the tile size
		// [false] if you want to pass the actual coord directly
		bool BySize				= true;
		glm::vec2 Offset		= { 0.0f, 0.0f };
		glm::vec2 Scale			= { 1.0f, 1.0f };

		SpriteComponent() = default;
		SpriteComponent(const SpriteComponent&) = default;
	};

	struct LineRendererComponent
	{
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		float Width = 1.0f;
		bool Primitive = true;

		glm::vec3 Start = { -0.5f, 0.0f, 0.0f };
		glm::vec3 End = { 0.5f, 0.0f, 0.0f };

		LineRendererComponent() = default;
		LineRendererComponent(const LineRendererComponent&) = default;
	};

	struct TextRendererComponent
	{
		std::string String;
		AssetHandle Font = DFFont::GetDefault();

		glm::vec4 Color{ 1.0f };
		glm::vec4 BgColor{ 0.0f };
		glm::vec4 CharBgColor{ 0.0f };

		bool DrawBg = false;
		float Kerning = 0.0f;
		float LineSpacing = 0.0f;

		TextRendererComponent() = default;
		TextRendererComponent(const TextRendererComponent&) = default;
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = true;
		glm::vec4 ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	// TODO: Correct this to use UUID instead of entt::entity, for better deserialization
	struct KTN_API HierarchyComponent
	{
		entt::entity Parent = entt::null;
		entt::entity First = entt::null;
		entt::entity Next = entt::null;
		entt::entity Prev = entt::null;
		uint32_t ChildCount = 0;

		// update hierarchy components when hierarchy component is added
		static void OnConstruct(entt::registry& p_Registry, entt::entity p_Entity);

		// update hierarchy components when hierarchy component is removed
		static void OnDestroy(entt::registry& p_Registry, entt::entity p_Entity);
		static void OnUpdate(entt::registry& p_Registry, entt::entity p_Entity);
		static void Reparent(entt::entity p_Entity, entt::entity p_Parent, entt::registry& p_Registry, HierarchyComponent& p_Hierarchy);

		HierarchyComponent() = default;
		HierarchyComponent(const HierarchyComponent&) = default;
		HierarchyComponent(entt::entity p_Parent) : Parent(p_Parent) {}
	};

	struct PhysicsBody2D
	{
		enum class Type { None = 0, Rigidbody, Character, Static };
		virtual Type GetType() const { return Type::None; }

		AssetHandle PhysicsMaterial2D = PhysicsMaterial2D::GetDefault();

		float Mass = 1.0f;
		bool IsTrigger = false;

		B2BodyID Body = {}; // Physics body id
	};

	#define PHYSICS_BODY_TYPE(type_enum) \
		static constexpr Type GetStaticType() { return Type::type_enum; } \
		Type GetType() const override { return GetStaticType(); }

	struct Rigidbody2DComponent : public PhysicsBody2D
	{
		PHYSICS_BODY_TYPE(Rigidbody)

		float LinearDamping = 0.0f;
		float AngularDamping = 0.1f;
		float GravityScale = 1.0f;
		bool FixedRotation = false;
		bool Sleeping = false;
		bool CanSleep = true;

		Rigidbody2DComponent() = default;
		Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
	};

	struct CharacterBody2DComponent : public PhysicsBody2D
	{
		PHYSICS_BODY_TYPE(Character)

		bool SlideOnCeiling = true;
		float FloorMaxAngle = glm::radians(45.0f);
		float FloorSnapLength = 1;
		float WallMinSlideAngle = glm::radians(15.0f);

		bool OnFloor = false;
		bool OnWall = false;
		bool OnCeiling = false;

		glm::vec2 UpDirection{ 0.0f, 1.0f };
		glm::vec2 FloorNormal{ 0.0f, 1.0f };

		CharacterBody2DComponent() { Mass = 0.0f; }
		CharacterBody2DComponent(const CharacterBody2DComponent&) = default;
	};

	struct StaticBody2DComponent : public PhysicsBody2D
	{
		PHYSICS_BODY_TYPE(Static)

		StaticBody2DComponent() { Mass = 0.0f; }
		StaticBody2DComponent(const StaticBody2DComponent&) = default;
	};

	enum class Shape2D
	{
		Rect = 0,
		Circle
	};

	struct BodyShape2DComponent
	{
		Shape2D Shape = Shape2D::Rect;

		glm::vec2 Offset = { 0.0f, 0.0f };
		glm::vec2 Size = { 0.5f, 0.5f }; // Size.x -> radius

		BodyShape2DComponent() = default;
		BodyShape2DComponent(const BodyShape2DComponent&) = default;
	};

	struct ScriptComponent
	{
		std::string FullClassName;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;
	};

	struct PrefabComponent
	{
		std::string Path = "";

		PrefabComponent() = default;
		PrefabComponent(const PrefabComponent&) = default;
	};

	using PhysicsBody2DTypes = entt::type_list<Rigidbody2DComponent, CharacterBody2DComponent, StaticBody2DComponent>;
	#define ALL_COMPONENTS IDComponent, TagComponent, TransformComponent, SpriteComponent, LineRendererComponent, TextRendererComponent, CameraComponent, HierarchyComponent, Rigidbody2DComponent, CharacterBody2DComponent, StaticBody2DComponent, BodyShape2DComponent, ScriptComponent, PrefabComponent

} // namespace KTN