#include "ktnpch.h"
#include "GLBase.h"
#include "GLShader.h"

// lib
#include <spirv_glsl.hpp>



namespace KTN
{
	namespace
	{
		static const int PUSHCONSTANT_BINDING = 64;

		static GLenum ShaderTypeToOpenGL(ShaderType p_Type)
		{
			switch (p_Type)
			{
				case ShaderType::Vertex:    		return GL_VERTEX_SHADER;
				case ShaderType::Fragment:  		return GL_FRAGMENT_SHADER;
				case ShaderType::Geometry: 			return GL_GEOMETRY_SHADER;
				case ShaderType::Compute: 			return GL_COMPUTE_SHADER;
				case ShaderType::TessControl: 		return GL_TESS_CONTROL_SHADER;
				case ShaderType::TessEvaluation:	return GL_TESS_EVALUATION_SHADER;
			}

			KTN_GLERROR("Unknown shader type!");
			return 0;
		}

		static bool CheckCompilerErrors(uint32_t p_Shader, const std::string_view& p_Type)
		{
			GLint success;
			GLchar infoLog[1024];
			if (p_Type != "PROGRAM")
			{
				GLCall(glGetShaderiv(p_Shader, GL_COMPILE_STATUS, &success));
				if (!success)
				{
					GLCall(glGetShaderInfoLog(p_Shader, 1024, NULL, infoLog));
					KTN_GLERROR("ERROR::SHADER_COMPILATION_ERROR");
				}
			}
			else
			{
				GLCall(glGetProgramiv(p_Shader, GL_LINK_STATUS, &success));
				if (!success)
				{
					GLCall(glGetProgramInfoLog(p_Shader, 1024, NULL, infoLog));
					KTN_GLERROR("ERROR::PROGRAM_LINKING_ERROR");
				}
			}

			return success;
		}

		static GLuint CreateProg(const ShaderSource& p_Source)
		{
			if (p_Source.empty())
				return 0;

			GLuint program = glCreateProgram();

			std::vector<GLuint> shaderIDs;
			for (auto&& [stage, source] : p_Source)
			{
				GLuint shaderID = shaderIDs.emplace_back(glCreateShader(ShaderTypeToOpenGL(stage)));
				auto source_char = source.data();
				GLCall(glShaderSource(shaderID, 1, &source_char, NULL));
				GLCall(glCompileShader(shaderID));
				if (!CheckCompilerErrors(shaderID, "SHADER"))
				{
					return 0;
				}

				GLCall(glAttachShader(program, shaderID));
			}

			GLCall(glLinkProgram(program));
			if (!CheckCompilerErrors(program, "PROGRAM"))
			{
				for (auto id : shaderIDs) {
					GLCall(glDeleteShader(id));
				}

				return 0;
			}

			for (auto id : shaderIDs)
			{
				GLCall(glDetachShader(program, id));
				GLCall(glDeleteShader(id));
			}

			return program;
		}

		static std::string_view ShaderStageToString(ShaderType p_Stage)
		{
			switch (p_Stage)
			{
				case ShaderType::Vertex:   			return "Vertex";
				case ShaderType::Fragment: 			return "Fragment";
				case ShaderType::TessControl: 		return "Tesselation Control";
				case ShaderType::TessEvaluation: 	return "Tesselation Evaluation";
				case ShaderType::Geometry: 			return "Geometry";
				case ShaderType::Compute: 			return "Compute";
			}

			KTN_GLERROR("Unknown shader stage")
			return "";
		}

	#ifndef KTN_DISABLE_SHADER_LOG
		#define SHADER_LOG(...) KTN_GLTRACE(__VA_ARGS__)
	#else
		#define SHADER_LOG(...)
	#endif // !KTN_DISABLE_SHADER_LOG


	} // namespace

	GLShader::GLShader(const SpirvSource& p_Source)
	{
		auto source = Reflect(p_Source);
		CreateProgram(source);

		for (int i = 0; i < m_PushConstants.size(); i++)
		{
			m_PushConstants[i].Data = new uint8_t[m_PushConstants[i].Size];
			m_PushConstantsBuffers.emplace_back(CreateRef<GLUniformBuffer>(m_PushConstants[i].Size));
		}
	}

	GLShader::~GLShader()
	{
		GLCall(glDeleteProgram(m_RendererID));
	}

	void GLShader::Bind() const
	{
		GLCall(glUseProgram(m_RendererID));
	}

	void GLShader::Unbind() const
	{
		GLCall(glUseProgram(0));
	}

	void GLShader::SetPushValue(const std::string& p_Name, void* p_Value)
	{
		for (auto& push : m_PushConstants)
			push.SetValue(p_Name, p_Value);
	}

	void GLShader::BindPushConstants(CommandBuffer* p_CommandBuffer)
	{
		for (int i = 0; i < m_PushConstants.size(); i++)
		{
			auto& push		= m_PushConstants[i];
			auto& buffer	= m_PushConstantsBuffers[i];

			buffer->SetData(push.Data, push.Size);
			buffer->Bind(PUSHCONSTANT_BINDING);
		}
	}

	ShaderSource GLShader::Reflect(const SpirvSource& p_Spirv)
	{
		ShaderSource shaderSources;

		SHADER_LOG("===================== SHADER REFLECT =====================");
		for (const auto& [stage, spirv] : p_Spirv)
		{
			spirv_cross::CompilerGLSL compiler(spirv);
			spirv_cross::ShaderResources resources = compiler.get_shader_resources();

			SHADER_LOG("Reflect - {}", ShaderStageToString(stage));
			SHADER_LOG("    {} uniform buffers", resources.uniform_buffers.size());
			SHADER_LOG("    {} storage buffers", resources.storage_buffers.size());
			SHADER_LOG("    {} sampled images", resources.sampled_images.size());
			SHADER_LOG("    {} storage images", resources.storage_images.size());

		#ifndef KTN_DISABLE_SHADER_LOG
			if (stage == ShaderType::Vertex)
			{
				SHADER_LOG(" Inputs: ")
				for (const auto& resource : resources.stage_inputs)
				{
					auto location = compiler.get_decoration(resource.id, spv::DecorationLocation);

					SHADER_LOG("  Name: {0}", resource.name)
					SHADER_LOG("    Location = {0}", location)
				}
			}
		#endif // !KTN_DISABLE_SHADER_LOG

			if (!resources.uniform_buffers.empty())
			{
				SHADER_LOG(" Uniform buffers: ");
				for (const auto& resource : resources.uniform_buffers)
				{
					const auto& bufferType					= compiler.get_type(resource.base_type_id);
					size_t bufferSize						= compiler.get_declared_struct_size(bufferType);
					uint32_t set							= compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
					uint32_t binding						= compiler.get_decoration(resource.id, spv::DecorationBinding);
					binding									= set * 16 + binding;
					compiler.set_decoration(resource.id, spv::DecorationBinding, binding);
					size_t memberCount						= bufferType.member_types.size();
					const auto& type						= compiler.get_type(resource.type_id);
					auto descriptorCount					= (type.array.size() > 0) ? type.array[0] : 1;

					SHADER_LOG("  Name: {0}", resource.name);
					SHADER_LOG("    Size = {0}", bufferSize);
					SHADER_LOG("    Set  = {0}", set);
					SHADER_LOG("    Binding = {0}", binding);
					SHADER_LOG("    Members = {0}", memberCount);
					SHADER_LOG("    Descriptor Count = {0}", descriptorCount);

					auto& descriptor						= m_DescriptorInfos[set].emplace_back();
					descriptor.Binding						= binding;
					descriptor.Size							= bufferSize;
					descriptor.Name							= resource.name;
					descriptor.Offset						= 0;
					descriptor.Stage						= stage;
					descriptor.Type							= DescriptorType::UNIFORM_BUFFER;

					for (size_t i = 0; i < memberCount; i++)
					{
						const auto& memberName				= compiler.get_member_name(bufferType.self, (uint32_t)i);
						auto size							= compiler.get_declared_struct_member_size(bufferType, (uint32_t)i);
						auto offset							= compiler.type_struct_member_offset(bufferType, (uint32_t)i);

						std::string uniformName				= resource.name + "." + memberName;

						auto& member						= descriptor.Members.emplace_back();
						member.Name							= memberName;
						member.FullName						= uniformName;
						member.Offset						= offset;
						member.Size							= size;
					}
				}
			}

			if (!resources.storage_buffers.empty())
			{
				SHADER_LOG(" Storage buffers: ");
				for (const auto& resource : resources.storage_buffers)
				{
					const auto& bufferType					= compiler.get_type(resource.base_type_id);
					size_t bufferSize						= compiler.get_declared_struct_size(bufferType);
					uint32_t set							= compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
					uint32_t binding						= compiler.get_decoration(resource.id, spv::DecorationBinding);
					binding									= set * 16 + binding;
					compiler.set_decoration(resource.id, spv::DecorationBinding, binding);
					auto memberCount						= (int)bufferType.member_types.size();
					const auto& type						= compiler.get_type(resource.type_id);
					auto descriptorCount					= (type.array.size() > 0) ? type.array[0] : 1;

					SHADER_LOG("  Name: {0}", resource.name);
					SHADER_LOG("    Size = {0}", bufferSize);
					SHADER_LOG("    Set  = {0}", set);
					SHADER_LOG("    Binding = {0}", binding);
					SHADER_LOG("    Members = {0}", memberCount);
					SHADER_LOG("    Descriptor Count = {0}", descriptorCount);

					auto& descriptor						= m_DescriptorInfos[set].emplace_back();
					descriptor.Binding						= binding;
					descriptor.Size							= bufferSize;
					descriptor.Name							= resource.name;
					descriptor.Offset						= 0;
					descriptor.Stage						= stage;
					descriptor.Type							= DescriptorType::STORAGE_BUFFER;

					for (int i = 0; i < memberCount; i++)
					{
						const auto& memberName				= compiler.get_member_name(bufferType.self, i);
						auto size							= compiler.get_declared_struct_member_size(bufferType, i);
						auto offset							= compiler.type_struct_member_offset(bufferType, i);

						std::string uniformName				= resource.name + "." + memberName;

						auto& member						= descriptor.Members.emplace_back();
						member.Name							= memberName;
						member.FullName						= uniformName;
						member.Offset						= offset;
						member.Size							= size;
					}
				}
			}

			if (!resources.sampled_images.empty())
			{
				SHADER_LOG(" Sampled images: ");
				for (const auto& resource : resources.sampled_images)
				{
					const auto& bufferType					= compiler.get_type(resource.base_type_id);
					uint32_t set							= compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
					uint32_t binding						= compiler.get_decoration(resource.id, spv::DecorationBinding);
					binding									= set * 16 + binding;
					compiler.set_decoration(resource.id, spv::DecorationBinding, binding);
					auto memberCount						= bufferType.member_types.size();
					const auto& Type						= compiler.get_type(resource.type_id);
					auto descriptorCount					= (Type.array.size() > 0) ? Type.array[0] : 1;

					SHADER_LOG("  Name: {0}", resource.name);
					SHADER_LOG("    Set  = {0}", set);
					SHADER_LOG("    Binding = {0}", binding);
					SHADER_LOG("    Members = {0}", memberCount);
					SHADER_LOG("    Descriptor Count = {0}", descriptorCount);

					auto& descriptor						= m_DescriptorInfos[set].emplace_back();
					descriptor.Binding						= binding;
					descriptor.Size							= descriptorCount;
					descriptor.Name							= resource.name;
					descriptor.Offset						= 0;
					descriptor.Stage						= stage;
					descriptor.Type							= DescriptorType::IMAGE_SAMPLER;
				}
			}

			if (!resources.storage_images.empty())
			{
				SHADER_LOG(" Storage images:");
				for (const auto& resource : resources.storage_images)
				{
					const auto& bufferType					= compiler.get_type(resource.base_type_id);
					uint32_t set							= compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
					uint32_t binding						= compiler.get_decoration(resource.id, spv::DecorationBinding);
					binding									= set * 16 + binding;
					compiler.set_decoration(resource.id, spv::DecorationBinding, binding);
					auto memberCount						= (int)bufferType.member_types.size();
					const auto& Type						= compiler.get_type(resource.type_id);
					auto descriptorCount					= (Type.array.size() > 0) ? Type.array[0] : 1;

					SHADER_LOG("  Name: {0}", resource.name);
					SHADER_LOG("    Set  = {0}", set);
					SHADER_LOG("    Binding = {0}", binding);
					SHADER_LOG("    Members = {0}", memberCount);
					SHADER_LOG("    Descriptor Count = {0}", descriptorCount);

					auto& descriptor						= m_DescriptorInfos[set].emplace_back();
					descriptor.Binding						= binding;
					descriptor.Size							= descriptorCount;
					descriptor.Name							= resource.name;
					descriptor.Offset						= 0;
					descriptor.Stage						= stage;
					descriptor.Type							= DescriptorType::STORAGE_IMAGE;
				}
			}

			if (!resources.push_constant_buffers.empty())
			{
				SHADER_LOG(" Push constant buffers:");
				for (const auto& resource : resources.push_constant_buffers)
				{
					const auto& bufferType					= compiler.get_type(resource.base_type_id);
					auto memberCount						= (uint32_t)bufferType.member_types.size();
					compiler.set_decoration(resource.id, spv::DecorationBinding, PUSHCONSTANT_BINDING);
					uint32_t binding						= compiler.get_decoration(resource.id, spv::DecorationBinding);
					std::string name						= resource.name;
					size_t size								= compiler.get_declared_struct_size(bufferType);

					SHADER_LOG("  Name: {0}", name);
					SHADER_LOG("    Binding = {0}", binding);
					SHADER_LOG("    Size = {0}", size);
					SHADER_LOG("    Members = {0}", memberCount);

					auto& push_constant						= m_PushConstants.emplace_back();
					push_constant.Name						= name;
					push_constant.Binding					= binding;
					push_constant.Offset					= 0;
					push_constant.Size						= size;
					push_constant.Stage						= stage;

					for (uint32_t i = 0; i < memberCount; i++)
					{
						std::string memberName				= compiler.get_member_name(resource.base_type_id, i);
						auto size							= compiler.get_declared_struct_member_size(bufferType, i);
						auto offset							= compiler.type_struct_member_offset(bufferType, i);

						auto& member						= push_constant.Members.emplace_back();
						member.FullName						= name.empty() ? memberName : name + "." + memberName;
						member.Name							= memberName;
						member.Offset						= offset;
						member.Size							= size;
					}
				}
			}

			spirv_cross::CompilerGLSL::Options options;
			options.version									= 450;
			options.es										= false;
			options.vulkan_semantics						= false;
			options.separate_shader_objects					= false;
			options.enable_420pack_extension				= false;
			options.emit_push_constant_as_uniform_buffer	= true;
			compiler.set_common_options(options);

			shaderSources[stage]							= compiler.compile();
		}
		SHADER_LOG("==========================================================")

		return shaderSources;
	}

	void GLShader::CreateProgram(const ShaderSource& p_Source)
	{
		ShaderSource shadersource;

		for (auto&& [stage, source] : p_Source)
		{
			if (stage == ShaderType::Compute)
			{
				m_IsCompute = true;
				shadersource[stage] = source;
				break;
			}
			else
			{
				shadersource[stage] = source;
			}
		}

		m_RendererID = 0;
		if (!shadersource.empty())
		{
			m_RendererID = CreateProg(shadersource);
			KTN_CORE_ASSERT(m_RendererID != 0);
		}
	}
}
