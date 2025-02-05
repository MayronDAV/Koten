#include "ktnpch.h"
#include "GraphicsContext.h"
#include "Koten/Core/Engine.h"

#include "Platform/OpenGL/GLContext.h"


namespace KTN
{
	Unique<GraphicsContext> GraphicsContext::Create()
	{
		if (Engine::GetAPI() == RenderAPI::OpenGL)
			return CreateUnique<GLContext>();

		KTN_CORE_ERROR("Unsupported API!");
		return nullptr;
	}

} // namespace KTN
