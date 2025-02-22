#include "ktnpch.h"
#include "Entity.h"



namespace KTN
{
	Entity::Entity(entt::entity p_Handle, Scene* p_Scene)
		: m_Handle(p_Handle), m_Scene(p_Scene)
	{
		KTN_PROFILE_FUNCTION();
	}

} // namespace KTN
