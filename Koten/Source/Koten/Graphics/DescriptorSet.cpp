#include "ktnpch.h"
#include "DescriptorSet.h"

#include "Platform/OpenGL/GLDescriptorSet.h"



namespace KTN
{
	Ref<DescriptorSet> DescriptorSet::Create(const DescriptorSetSpecification& p_Spec)
	{
		KTN_PROFILE_FUNCTION();

		if (Engine::GetAPI() == RenderAPI::OpenGL)
			return CreateRef<GLDescriptorSet>(p_Spec);

		KTN_CORE_ERROR("Unsupported API!");
		return nullptr;
	}

} // namespace KTN
