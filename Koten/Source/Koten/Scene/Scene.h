#pragma once
#include "Koten/Core/Base.h"
#include "Components.h"

// lib
#include <entt/entt.hpp>

// std
#include <string>



namespace KTN
{
	class KTN_API Scene
	{
		public:
			Scene();
			~Scene();

			entt::entity CreateEntity();

			void OnUpdate();
			void OnRender();

			entt::registry& GetRegistry() { return m_Registry; }

		private:
			entt::registry m_Registry;
	};


} // namespace KTN