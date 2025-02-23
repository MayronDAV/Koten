#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Graphics/Texture.h"
#include "Koten/Graphics/Camera.h"

// lib
#include <glm/glm.hpp>



namespace KTN
{
	struct RenderCommand 
	{
		enum class Type 
		{
			Sprite2D
		};
		Type pType = Type::Sprite2D;

		glm::mat4 Transform			= { 1.0f };

		struct 
		{
			Ref<Texture2D> Texture  = nullptr;
			glm::vec4 Color			= { 1.0f, 1.0f, 1.0f, 1.0f };

			// UV Options

			glm::vec2 Size			= { 0.0f, 0.0f };
			// [true] if you want to pass the tile coord as a multiplier of the tile size
			// [false] if you want to pass the actual coord directly
			bool BySize				= true;
			glm::vec2 Offset		= { 0.0f, 0.0f };
			glm::vec2 Scale			= { 1.0f, 1.0f };
		} SpriteData = {};
	};

	struct RenderBeginInfo
	{
		Ref<Texture2D> RenderTarget = nullptr;
		uint32_t Width				= 0;
		uint32_t Height				= 0;
		uint8_t Samples				= 1;
		glm::mat4 Projection		= {};
		glm::mat4 View				= { 1.0f };
		glm::vec4 ClearColor		= { 0.0f, 0.0f, 0.0f, 1.0f };
		bool Clear					= true;
	};

	class KTN_API Renderer
	{
		public:
			static void Init();
			static void Shutdown();

			static void Clear();

			static void Begin(const RenderBeginInfo& p_Info);
			static void End();

			static void Submit(const RenderCommand& p_Command);
	};

} // namespace KTN