#pragma once
#include "Koten/Graphics/DescriptorSet.h"


namespace KTN
{
	class GLDescriptorSet : public DescriptorSet
	{
		public:
			GLDescriptorSet(const DescriptorSetSpecification& p_Spec);
			~GLDescriptorSet() override = default;

			void SetUniformData(const std::string& p_Name, const Ref<UniformBuffer>& p_UniformBuffer) override;
			void SetUniformData(const std::string& p_Name, void* p_Data) override;
			void SetUniformData(const std::string& p_Name, void* p_Data, size_t p_Size) override;

			void SetUniform(const std::string& p_BufferName, const std::string& p_MemberName, void* p_Data) override;
			void SetUniform(const std::string& p_BufferName, const std::string& p_MemberName, void* p_Data, size_t p_Size) override;

			void PrepareStorageBuffer(const std::string& p_Name, size_t p_Size) override;

			void SetStorageData(const std::string& p_Name, const Ref<StorageBuffer>& p_StorageBuffer) override;
			void SetStorageData(const std::string& p_Name, void* p_Data) override;
			void SetStorageData(const std::string& p_Name, void* p_Data, size_t p_Size) override;

			void SetStorage(const std::string& p_BufferName, const std::string& p_MemberName, void* p_Data) override;
			void SetStorage(const std::string& p_BufferName, const std::string& p_MemberName, void* p_Data, size_t p_Size) override;

			void SetTexture(const std::string& p_Name, const Ref<Texture2D>& p_Texture) override;
			void SetTexture(const std::string& p_Name, const Ref<Texture2D>* p_TextureData, uint32_t p_Count) override;

			void Upload(CommandBuffer* p_CommandBuffer) override;

		protected:
			void Bind(CommandBuffer* p_CommandBuffer) override;

		private:
			std::vector<DescriptorInfo> m_DescriptorsInfo;
			std::vector<int> m_Queue;
	};
} // namespace KTN