#pragma once
#include "Koten/Core/Log.h"

// lib
#include <glm/glm.hpp>

// std
#include <string>
#include <cstdint>




namespace KTN
{
	#pragma region defs
		class Texture2D;
		class StorageBuffer;
		class UniformBuffer;
		class Framebuffer;
		class Renderpass;
		class CommandBuffer;
		class Shader;

		struct B2WorldID
		{
			uint16_t Index;
			uint16_t Generation;
		};

		struct B2BodyID
		{
			int32_t Index;
			uint16_t World;
			uint16_t Generation;

			inline bool operator== (const B2BodyID& p_Other) const
			{
				return Index == p_Other.Index &&
					World == p_Other.World &&
					Generation == p_Other.Generation;
			}

			inline bool operator== (B2BodyID& p_Other) const
			{
				return Index == p_Other.Index &&
					World == p_Other.World &&
					Generation == p_Other.Generation;
			}
		};

		inline constexpr uint8_t MAX_RENDER_TARGETS = 4;

	#pragma endregion

	#pragma region Enums

	enum class RenderAPI : uint8_t
	{
		None = 0, OpenGL, Vulkan
	};

	enum class WindowMode : uint8_t
	{
		Windowed = 0,
		Fullscreen,
		Borderless
	};

	enum class CursorMode : uint8_t
	{
		Normal = 0, Hidden, Disabled
	};

	enum class ShaderType : uint8_t
	{
		Vertex = 0,
		Fragment,
		Compute,
		Geometry,
		TessControl,
		TessEvaluation
	};

	enum class DescriptorType : uint8_t
	{
		UNIFORM_BUFFER,
		STORAGE_BUFFER,
		STORAGE_IMAGE,
		IMAGE_SAMPLER
	};

	enum class DataType : uint8_t
	{
		None = 0,
		Float, Float2, Float3, Float4,
		Float3x3, Float4x4,
		UInt, UInt2, UInt3, UInt4,
		UInt3x3, UInt4x4,
		Int, Int2, Int3, Int4,
		Int3x3, Int4x4,
		Bool
	};

	enum class TextureUsage : uint8_t
	{
		TEXTURE_SAMPLED = 0,
		TEXTURE_STORAGE,
		TEXTURE_COLOR_ATTACHMENT,
		TEXTURE_DEPTH_STENCIL_ATTACHMENT,
	};

	enum class TextureWrap : uint8_t
	{
		NONE = 0,
		REPEAT,
		MIRRORED_REPEAT,
		CLAMP_TO_EDGE,
		CLAMP_TO_BORDER
	};

	enum class TextureFilter : uint8_t
	{
		NONE = 0,
		LINEAR,
		NEAREST
	};

	enum class TextureAccess : uint8_t
	{
		READ_WRITE = 0,
		READ_ONLY,
		WRITE_ONLY
	};

	enum class TextureFormat : uint8_t
	{
		NONE = 0,

		// Color Format

		R8,
		R8_INT,
		R8_UINT,
		R32_INT,
		R32_UINT,
		R32_FLOAT,
		R16_FLOAT,

		RG8,
		RG32_UINT,
		RG16_FLOAT,

		RGB8,

		RGBA8,
		RGBA16_FLOAT,
		RGBA32_FLOAT,

		// Depth Format

		D16,
		D32_FLOAT,

		// DepthStencil Format

		D16_S8_UINT,
		D24_S8_UINT,
		D32_FLOAT_S8_UINT,
	};

	enum SubpassContents : uint8_t
	{
		INLINE = 0,
		SECONDARY
	};

	enum class CullMode : uint8_t
	{
		NONE = 0,
		FRONT,
		BACK,
		FRONTANDBACK
	};

	enum class PolygonMode : uint8_t
	{
		FILL = 0,
		LINE,
		POINT
	};

	enum class BlendMode : uint8_t
	{
		None = 0,
		OneZero,
		ZeroSrcColor,
		SrcAlphaOneMinusSrcAlpha,
		SrcAlphaOne,
		OneOne,
		ZeroOneMinusSrcColor
	};

	enum class DrawType : uint8_t
	{
		TRIANGLES = 0,
		POINTS,
		LINES
	};

	enum class FrontFace : uint8_t
	{
		CLOCKWISE = 0,
		COUNTER_CLOCKWISE
	};

	enum class StencilFace : uint8_t
	{
		FRONT_AND_BACK = 0,
		FRONT,
		BACK
	};

	enum class StencilCompare : uint8_t
	{
		ALWAYS = 0,
		NEVER,
		LESS,
		EQUAL,
		LEQUAL,
		GREATER,
		NOTEQUAL,
		GEQUAL
	};

	enum class StencilOp : uint8_t
	{
		KEEP = 0,
		ZERO,
		REPLACE,
		INCR,
		DECR,
		INVERT,
		INCR_WRAP,
		DECR_WRAP
	};

	enum class RenderType2D : uint8_t
	{
		Quad = 0,
		Circle
	};

	enum class ScriptFieldType : uint8_t
	{
		None = 0,
		Float, Double,
		Bool, Char, Byte, Short, Int, Long,
		UByte, UShort, UInt, ULong,
		String,
		Vector2, Vector3, Vector4,
		Entity
	};

	#pragma endregion

	#pragma region Structs

	struct WindowResolution
	{
		int Width;
		int Height;
		int RefreshRate;
	};

	struct WindowSpecification
	{
		std::string Title		= "Koten";
		uint32_t Width			= 800;
		uint32_t Height			= 600;
		WindowMode Mode			= WindowMode::Windowed;
		bool Resizable			= true;
		bool Maximize			= false;
		bool Center				= true;
		bool Vsync				= false;
		std::string IconPath	= "";
	};

	struct MemberInfo
	{
		size_t Size;
		size_t Offset;
		std::string Name;
		std::string FullName;
	};

	struct DescriptorInfo
	{
		std::vector<Ref<Texture2D>> Textures;
		Ref<StorageBuffer> Storage = nullptr;
		Ref<UniformBuffer> Uniform = nullptr;

		std::string Name;
		uint32_t Binding = 0u;
		size_t Offset = 0ul;
		size_t Size;
		DescriptorType Type;
		ShaderType Stage;

		std::vector<MemberInfo> Members;
	};

	struct PushConstant
	{
		std::string Name;
		uint32_t Binding = 0u;
		uint8_t* Data;
		size_t Size;
		size_t Offset = 0ul;
		ShaderType Stage;

		std::vector<MemberInfo> Members;

		inline void SetValue(const std::string& p_Name, void* p_Value)
		{
			for (auto& member : Members)
			{
				if (member.Name == p_Name)
				{
					memcpy(Data + member.Offset, p_Value, member.Size);
					return;
				}
			}

			KTN_CORE_ERROR("Push Constant not found: {}", p_Name);
		}

		~PushConstant()
		{
			if (Data)
			{
				delete Data;
				Data = nullptr;
			}
		}
	};

	struct TextureSpecification
	{
		uint32_t Width					= 1;
		uint32_t Height					= 1;
		TextureUsage Usage				= TextureUsage::TEXTURE_SAMPLED;
		TextureAccess Access			= TextureAccess::READ_WRITE; // TEXTURE_STORAGE
		TextureFormat Format			= TextureFormat::RGBA8;
		TextureFilter MinFilter			= TextureFilter::LINEAR;
		TextureFilter MagFilter			= TextureFilter::LINEAR;
		TextureWrap WrapU				= TextureWrap::REPEAT;
		TextureWrap WrapV				= TextureWrap::REPEAT;
		TextureWrap WrapW				= TextureWrap::REPEAT;

		glm::vec4 BorderColor			= { 0.0f, 0.0f, 0.0f, 1.0f };
		int Samples						= 1;

		bool SRGB						= true;
		bool AnisotropyEnable			= true;
		bool GenerateMips				= true;

		std::string DebugName;
	};

	struct RenderpassSpecification
	{
		Ref<Texture2D>* Attachments		= nullptr;
		uint32_t AttachmentCount		= 0;
		bool Clear						= true;
		bool SwapchainTarget			= false;
		int Samples						= 1;
		Ref<Texture2D> ResolveTexture	= nullptr;

		std::string DebugName;
	};

	struct FramebufferSpecification
	{
		Ref<Renderpass> RenderPass		= nullptr;
		Ref<Texture2D>* Attachments		= nullptr;
		uint32_t AttachmentCount		= 0;
		uint32_t Width					= 0;
		uint32_t Height					= 0;
		uint32_t Samples				= 1;
		int MipIndex					= 0;
		bool SwapchainTarget			= false;

		std::string DebugName;
	};

	struct PipelineSpecification
	{
		Ref<Shader> pShader				= nullptr;

		std::array<Ref<Texture2D>, MAX_RENDER_TARGETS> ColorTargets;
		std::array<BlendMode, MAX_RENDER_TARGETS> BlendModes;

		Ref<Texture2D> ResolveTexture	= nullptr;
		Ref<Texture2D> DepthTarget		= nullptr;

		CullMode pCullMode				= CullMode::NONE;
		FrontFace pFrontFace			= FrontFace::CLOCKWISE;
		PolygonMode pPolygonMode		= PolygonMode::FILL;
		DrawType pDrawType				= DrawType::TRIANGLES;

		bool TransparencyEnabled		= false;
		bool ClearTargets				= true;
		bool DepthTest					= true;
		bool DepthWrite					= true;
		bool SwapchainTarget			= false;
		bool DepthBiasEnabled			= false;
		bool StencilTest				= false;
		bool BuildMipFramebuffers		= false;

		float LineWidth					= 1.0f;
		float ConstantFactor			= 0.0f;
		float SlopeFactor				= 0.0f;
		glm::vec4 ClearColor			= { 1.0f, 1.0f, 1.0f, 1.0f };
		int Samples						= 1;

		StencilFace pStencilFace		= StencilFace::FRONT_AND_BACK;
		StencilCompare pStencilCompare  = StencilCompare::ALWAYS;
		StencilOp FailOp				= StencilOp::KEEP;
		StencilOp DepthFailOp			= StencilOp::KEEP;
		StencilOp PassOp				= StencilOp::KEEP;
		int StencilReference			= 1;
		uint32_t StencilCompareMask 	= 0xFF;
		uint32_t StencilWriteMask    	= 0xFF;

		std::string DebugName			= "Pipeline";
	};

	#pragma endregion

} // namespace KTN