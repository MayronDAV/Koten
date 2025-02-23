#include "ktnpch.h"
#include "Entity.h"



namespace KTN
{
	Entity::Entity(entt::entity p_Handle, Scene* p_Scene)
		: m_Handle(p_Handle), m_Scene(p_Scene)
	{
		KTN_PROFILE_FUNCTION();
	}

	std::string Entity::GetName() const
	{
		return m_Scene->m_Registry.get<TagComponent>(m_Handle).Tag;
	}

	void Entity::Destroy()
	{
		m_Scene->GetRegistry().destroy(m_Handle);
		m_Scene = nullptr;
		m_Handle = entt::null;
	}

} // namespace KTN
