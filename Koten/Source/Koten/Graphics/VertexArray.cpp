#include "ktnpch.h"
#include "VertexArray.h"

#include "Platform/OpenGL/GLVertexArray.h"



namespace KTN
{
	Ref<VertexArray> VertexArray::Create()
	{
		KTN_PROFILE_FUNCTION();

		if (Engine::GetAPI() == RenderAPI::OpenGL)
			return CreateRef<GLVertexArray>();

		KTN_CORE_ERROR("Unsupported API!");
		return nullptr;
	}
}
