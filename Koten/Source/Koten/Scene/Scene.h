#pragma once
#include "Koten/Core/Base.h"
#include "Components.h"

// lib
#include <entt/entt.hpp>

// std
#include <string>



namespace KTN
{
	class KTN_API Entity;

	class KTN_API Scene
	{
		public:
			Scene();
			Scene(const Scene&) = default;
			~Scene();

			Entity CreateEntity(const std::string& p_Tag);

			void OnUpdate();
			void OnRender();

		private:
			entt::registry m_Registry;

			friend class Entity;
	};


} // namespace KTN