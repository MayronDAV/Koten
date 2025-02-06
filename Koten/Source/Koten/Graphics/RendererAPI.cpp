#include "ktnpch.h"
#include "RendererAPI.h"

#include "Platform/OpenGL/GLRendererAPI.h"


namespace KTN
{
	RendererAPI* RendererAPI::Create()
	{
		if (Engine::GetAPI() == RenderAPI::OpenGL)
			return new GLRendererAPI();

		KTN_CORE_ERROR("Unsupported API!");
		return nullptr;
	}

} // namespace KTN
