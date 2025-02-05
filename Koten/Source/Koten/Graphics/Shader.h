#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Core/Definitions.h"

// std
#include <unordered_map>
#include <vector>



namespace KTN
{
	using ShaderSource		= std::unordered_map<ShaderType, std::string>;
	using SpirvSource		= std::unordered_map<ShaderType, std::vector<uint32_t>>;
	using DescriptorInfos	= std::unordered_map<uint16_t, std::vector<DescriptorInfo>>;
	using PushConstants		= std::vector<PushConstant>;

	class KTN_API CommandBuffer;

	class KTN_API Shader
	{
		public:
			virtual ~Shader() = default;

			virtual void Bind() const = 0;
			virtual void Unbind() const = 0;

			virtual bool IsCompute() const = 0;

			virtual void SetPushValue(const std::string& p_Name, void* p_Value) = 0;
			virtual void BindPushConstants(CommandBuffer* p_CommandBuffer) = 0;

			virtual const std::vector<DescriptorInfo>& GetDescriptorInfos(uint16_t p_Key) = 0;

			static Ref<Shader> Create(const std::string& p_Path);

		private:
			static std::string ProcessIncludeFiles(const std::string& p_Path, const std::string& p_Code);
			static ShaderSource Process(const std::string& p_Path);

			static SpirvSource CompileOrGetSpirv(const std::string& p_Path);
	};

} // namespace KTN
