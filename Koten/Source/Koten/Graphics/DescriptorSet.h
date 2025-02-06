#pragma once
#include "Koten/Core/Base.h"
#include "UniformBuffer.h"
#include "Texture.h"
#include "Shader.h"



namespace KTN
{
	class KTN_API CommandBuffer;

	struct DescriptorSetSpecification
	{
		uint32_t Set		= 0;
		Ref<Shader> pShader = nullptr;
	};

	class KTN_API DescriptorSet
	{
		friend class CommandBuffer;
		friend class GLCommandBuffer;

		public:
			virtual ~DescriptorSet() = default;

			virtual void SetUniformData(const std::string& p_Name, const Ref<UniformBuffer>& p_UniformBuffer) = 0;
			virtual void SetUniformData(const std::string& p_Name, void* p_Data) = 0;
			virtual void SetUniformData(const std::string& p_Name, void* p_Data, size_t p_Size) = 0;

			virtual void SetUniform(const std::string& p_BufferName, const std::string& p_MemberName, void* p_Data) = 0;
			virtual void SetUniform(const std::string& p_BufferName, const std::string& p_MemberName, void* p_Data, size_t p_Size) = 0;

			virtual void SetTexture(const std::string& p_Name, const Ref<Texture2D>& p_Texture) = 0;
			virtual void SetTexture(const std::string& p_Name, const Ref<Texture2D>* p_TextureData, uint32_t p_Count) = 0;

			virtual void Upload(CommandBuffer* p_CommandBuffer) = 0;

			static Ref<DescriptorSet> Create(const DescriptorSetSpecification& p_Spec);

		protected:
			virtual void Bind(CommandBuffer* p_CommandBuffer) = 0;
	};
} // namespace KTN