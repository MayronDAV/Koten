#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Math/Transform.h"
#include "Koten/Graphics/Texture.h"
#include "Koten/Graphics/Camera.h"
#include "Koten/Core/UUID.h"

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
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Texture2D> Texture = nullptr;

		SpriteComponent() = default;
		SpriteComponent(const SpriteComponent&) = default;
		SpriteComponent(const glm::vec4& p_Color)
			: Color(p_Color) {}
		SpriteComponent(const glm::vec4& p_Color, const Ref<Texture2D>& p_Texture)
			: Color(p_Color), Texture(p_Texture) {}
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = true;
		glm::vec4 ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

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

	struct Rigidbody2DComponent
	{
		enum class BodyType
		{
			Static = 0,
			Dynamic,
			Kinematic
		};
		BodyType Type = BodyType::Static;
		bool FixedRotation = false;

		B2BodyID Body = {}; // Physics body id

		Rigidbody2DComponent() = default;
		Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
	};

	struct BoxCollider2DComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		glm::vec2 Size = { 0.5f, 0.5f };

		float Density = 1;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
	};

	#define ALL_COMPONENTS TagComponent, TransformComponent, SpriteComponent, CameraComponent, HierarchyComponent, Rigidbody2DComponent

} // namespace KTN