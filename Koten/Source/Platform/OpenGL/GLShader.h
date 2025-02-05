#pragma once
#include "Koten/Graphics/Shader.h"



namespace KTN
{
	class GLShader : public Shader
	{
		public:
			GLShader(const SpirvSource& p_Source);
			~GLShader() override;

			void Bind() const override;
			void Unbind() const override;

			void SetPushValue(const std::string& p_Name, void* p_Value) override;
			void BindPushConstants(CommandBuffer* p_CommandBuffer) override;


			const std::vector<DescriptorInfo>& GetDescriptorInfos(uint16_t p_Key) override { return m_DescriptorInfos[p_Key]; }

			bool IsCompute() const override { return m_IsCompute; }
			uint32_t GetID() const { return m_RendererID; }

		private:
			ShaderSource Reflect(const SpirvSource& p_Spirv);
			void CreateProgram(const ShaderSource& p_Source);

		private:
			uint32_t m_RendererID = 0;
			bool m_IsCompute = false;
			PushConstants m_PushConstants;
			DescriptorInfos m_DescriptorInfos;
	};

} // namespace KTN
