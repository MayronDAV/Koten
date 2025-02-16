#include "ktnpch.h"
#include "GLBase.h"
#include "GLDescriptorSet.h"
#include "GLUniformBuffer.h"
#include "GLTexture.h"




namespace KTN
{
	GLDescriptorSet::GLDescriptorSet(const DescriptorSetSpecification& p_Spec)
	{
		KTN_PROFILE_FUNCTION_LOW();

		m_DescriptorsInfo = p_Spec.pShader->GetDescriptorInfos(p_Spec.Set);

		for (auto& info : m_DescriptorsInfo)
		{
			if (info.Type == DescriptorType::UNIFORM_BUFFER)
			{
				info.Uniform = UniformBuffer::Create(info.Size);
			}
		}
	}

	void GLDescriptorSet::SetUniformData(const std::string& p_Name, const Ref<UniformBuffer>& p_UniformBuffer)
	{
		KTN_PROFILE_FUNCTION_LOW();

		for (size_t i = 0; i < m_DescriptorsInfo.size(); i++)
		{
			auto& descriptor = m_DescriptorsInfo[i];
			if (descriptor.Name == p_Name && descriptor.Type == DescriptorType::UNIFORM_BUFFER)
			{
				// explicit reset
				if (descriptor.Uniform)
					descriptor.Uniform.reset();

				descriptor.Uniform = p_UniformBuffer;
				m_Queue.push_back((int)i);
				return;
			}
		}

		KTN_CORE_ERROR("Unkown name {}", p_Name);
	}

	void GLDescriptorSet::SetUniformData(const std::string& p_Name, void* p_Data)
	{
		KTN_PROFILE_FUNCTION_LOW();

		for (size_t i = 0; i < m_DescriptorsInfo.size(); i++)
		{
			auto& descriptor = m_DescriptorsInfo[i];
			if (descriptor.Name == p_Name && descriptor.Type == DescriptorType::UNIFORM_BUFFER)
			{
				descriptor.Uniform->SetData(p_Data, descriptor.Size);
				m_Queue.push_back((int)i);
				return;
			}
		}

		KTN_CORE_ERROR("Unkown name {}", p_Name);
	}

	void GLDescriptorSet::SetUniformData(const std::string& p_Name, void* p_Data, size_t p_Size)
	{
		KTN_PROFILE_FUNCTION_LOW();

		for (size_t i = 0; i < m_DescriptorsInfo.size(); i++)
		{
			auto& descriptor = m_DescriptorsInfo[i];
			if (descriptor.Name == p_Name && descriptor.Type == DescriptorType::UNIFORM_BUFFER)
			{
				if (p_Size != descriptor.Size)
				{
					descriptor.Uniform.reset();
					descriptor.Uniform = UniformBuffer::Create(p_Size);
				}

				descriptor.Uniform->SetData(p_Data, p_Size);
				m_Queue.push_back((int)i);
				return;
			}
		}

		KTN_CORE_ERROR("Unkown name {}", p_Name);
	}

	void GLDescriptorSet::SetUniform(const std::string& p_BufferName, const std::string& p_MemberName, void* p_Data)
	{
		KTN_PROFILE_FUNCTION_LOW();

		for (size_t i = 0; i < m_DescriptorsInfo.size(); i++)
		{
			auto& descriptor = m_DescriptorsInfo[i];
			if (descriptor.Name == p_BufferName && descriptor.Type == DescriptorType::UNIFORM_BUFFER)
			{
				for (const auto& member : descriptor.Members)
				{
					if (member.Name == p_MemberName)
					{
						descriptor.Uniform->SetData(p_Data, member.Size, (size_t)member.Offset);
						if (member.Offset == 0)
							m_Queue.push_back((int)i);
						return;
					}
				}
			}
		}

		KTN_CORE_ERROR("Unkown buffer name {} or member name {}", p_BufferName, p_MemberName);
	}

	void GLDescriptorSet::SetUniform(const std::string& p_BufferName, const std::string& p_MemberName, void* p_Data, size_t p_Size)
	{
		KTN_PROFILE_FUNCTION_LOW();

		for (size_t i = 0; i < m_DescriptorsInfo.size(); i++)
		{
			auto& descriptor = m_DescriptorsInfo[i];
			if (descriptor.Name == p_BufferName && descriptor.Type == DescriptorType::UNIFORM_BUFFER)
			{
				for (const auto& member : descriptor.Members)
				{
					if (member.Name == p_MemberName)
					{
						descriptor.Uniform->SetData(p_Data, p_Size, (size_t)member.Offset);
						if (member.Offset == 0)
							m_Queue.push_back((int)i);
						return;
					}
				}
			}
		}

		KTN_CORE_ERROR("Unkown buffer name {} or member name {}", p_BufferName, p_MemberName);
	}

	void GLDescriptorSet::SetTexture(const std::string& p_Name, const Ref<Texture2D>& p_Texture)
	{
		KTN_PROFILE_FUNCTION_LOW();

		for (size_t i = 0; i < m_DescriptorsInfo.size(); i++)
		{
			auto& descriptor = m_DescriptorsInfo[i];
			if (descriptor.Name == p_Name &&
				(descriptor.Type == DescriptorType::IMAGE_SAMPLER ||
					descriptor.Type == DescriptorType::STORAGE_IMAGE))
			{
				descriptor.Textures.clear();
				descriptor.Textures.push_back(p_Texture);
				m_Queue.push_back((int)i);
				return;
			}
		}

		KTN_CORE_ERROR("Unkown name {}", p_Name);
	}

	void GLDescriptorSet::SetTexture(const std::string& p_Name, const Ref<Texture2D>* p_TextureData, uint32_t p_Count)
	{
		KTN_PROFILE_FUNCTION_LOW();

		for (size_t i = 0; i < m_DescriptorsInfo.size(); i++)
		{
			auto& descriptor = m_DescriptorsInfo[i];
			if (descriptor.Name == p_Name &&
				(descriptor.Type == DescriptorType::IMAGE_SAMPLER ||
					descriptor.Type == DescriptorType::STORAGE_IMAGE))
			{
				descriptor.Textures.clear();
				descriptor.Textures.resize(p_Count);
				for (uint32_t j = 0; j < p_Count; j++)
				{
					descriptor.Textures[j] = p_TextureData[j];
				}
				m_Queue.push_back((int)i);
				return;
			}
		}

		KTN_CORE_ERROR("Unkown name {}", p_Name);
	}

	void GLDescriptorSet::Upload(CommandBuffer* p_CommandBuffer)
	{
	}

	void GLDescriptorSet::Bind(CommandBuffer* p_CommandBuffer)
	{
		KTN_PROFILE_FUNCTION_LOW();

		for (int index : m_Queue)
		{
			const auto& info = m_DescriptorsInfo[index];

			if (info.Type == DescriptorType::UNIFORM_BUFFER)
			{
				As<UniformBuffer, GLUniformBuffer>(info.Uniform)->Bind(info.Binding);
			}
			else if (info.Type == DescriptorType::IMAGE_SAMPLER || info.Type == DescriptorType::STORAGE_IMAGE)
			{
				for (int i = 0; i < info.Textures.size(); i++)
				{
					if (info.Textures[i] != nullptr)
					{
						As<Texture2D, GLTexture2D>(info.Textures[i])->Bind(info.Binding + i);
					}
				}
			}
		}

		m_Queue.clear();
	}

} // namespace KTN
