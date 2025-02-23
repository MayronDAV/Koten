#pragma once
#include "Koten/Core/Base.h"
#include "Components.h"

// lib
#include <entt/entt.hpp>



namespace KTN
{
	class KTN_API SceneGraph
	{
		public:
			SceneGraph();
			~SceneGraph() = default;

			void Init(entt::registry& p_Registry);

			void DisableOnConstruct(entt::registry& p_Registry, bool p_Disable);

			void Update(entt::registry& p_Registry);
			void UpdateTransform(entt::registry& p_Registry, entt::entity p_Entity);
	};

} // namespace KTN