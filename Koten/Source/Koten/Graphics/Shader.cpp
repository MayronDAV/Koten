#include "ktnpch.h"
#include "Shader.h"

#include "Platform/OpenGL/GLShader.h"

// lib
#include <SPIRV/GlslangToSpv.h>
#include <glslang/Public/ShaderLang.h>



namespace KTN
{
	namespace
	{
		static TBuiltInResource DefaultTBuiltInResource = {
			32,  // maxLights
			6,   // maxClipDistances
			8,   // maxCullDistances
			1,   // maxCombinedClipAndCullDistances
			2048, // maxCombinedShaderOutputResources
			4096, // maxComputeSharedMemorySize
			16,  // maxComputeWorkGroupCount
			1024, // maxComputeWorkGroupSize
			8,   // maxFragmentInputComponents
			64,  // maxImageUnits
			128, // maxImageSamples
			8,   // maxVertexOutputComponents
			8,   // maxTessControlOutputComponents
			16,  // maxTessEvaluationOutputComponents
			8,   // maxGeometryOutputComponents
			64,  // maxFragmentOutputAttachments
			8,   // maxGeometryInputComponents
			8,   // maxGeometryOutputComponents
			256, // maxFragmentCombinedOutputResources
			64,  // maxComputeWorkGroupInvocations
			16,  // maxWorkGroupSize
			1024, // maxWorkGroupCount
			8,   // maxGeometryOutputVertices
			1024, // maxGeometryTotalOutputComponents
			16,  // maxFragmentInputComponents
			16,  // maxVertexInputComponents
			1024, // maxTessControlInputComponents
			1024, // maxTessEvaluationInputComponents
			256, // maxTessControlOutputComponents
			1024, // maxTessEvaluationOutputComponents
			2048, // maxShaderStorageBufferBindings
			2048, // maxShaderStorageBufferSize
			128, // maxAtomicCounterBindings
			1024, // maxAtomicCounterBufferSize
			32,  // maxShaderImageSize
			2048, // maxShaderResourceSize
			64,  // maxShaderSamplerSize
			8,   // maxShaderConstantSize
			8,   // maxShaderPushConstantSize
			1024, // maxShaderUniformBufferSize
			128, // maxShaderStorageBufferSize
			1024, // maxShaderAtomicCounterSize
			256, // maxShaderAtomicCounterBindings
			2048, // maxShaderStorageBufferBindings
			256, // maxShaderStorageBufferSize
			2048, // maxShaderResourceSize
			128, // maxShaderSamplerSize
			128, // maxShaderSampledImageSize
			64,  // maxShaderImageSize
			8,   // maxShaderConstantSize
			16,  // maxShaderPushConstantSize
			256, // maxShaderUniformBufferSize
			2048, // maxShaderStorageBufferBindings
			256, // maxShaderStorageBufferSize
			2048, // maxShaderResourceSize
			128, // maxShaderSamplerSize
			128, // maxShaderSampledImageSize
			64,  // maxShaderImageSize
			8,   // maxShaderConstantSize
			16,  // maxShaderPushConstantSize
		};

		static std::string s_CacheDirectory = "Assets/Shaders/Cache/";

		static ShaderType ShaderTypeFromString(const std::string& p_Type)
		{
			if (p_Type == "vertex" || p_Type == "vert")
				return ShaderType::Vertex;
			if (p_Type == "fragment" || p_Type == "frag")
				return ShaderType::Fragment;
			if (p_Type == "geometry" || p_Type == "geom")
				return ShaderType::Geometry;
			if (p_Type == "tesselation control" || p_Type == "tesc")
				return ShaderType::TessControl;
			if (p_Type == "tesselation evaluation" || p_Type == "tese")
				return ShaderType::TessEvaluation;

			KTN_CORE_ERROR("Unknown shader type string!")
			return ShaderType(0);
		}

		static EShLanguage ShaderTypeToGlslang(ShaderType p_Type)
		{
			switch (p_Type)
			{
				case ShaderType::Vertex:   				return EShLangVertex;
				case ShaderType::Fragment: 				return EShLangFragment;
				case ShaderType::Geometry: 				return EShLangGeometry;
				case ShaderType::TessControl:			return EShLangTessControl;
				case ShaderType::TessEvaluation:		return EShLangTessEvaluation;
				case ShaderType::Compute: 				return EShLangCompute;
				default:
					KTN_CORE_ERROR("Unknown shader type!")
					return EShLangCount;
			}
		}

		static std::string ShaderTypeCachedFileExtension(ShaderType p_Type)
		{
			std::string prefix = "";
			if (Engine::Get().GetAPI() == RenderAPI::OpenGL)
				prefix = ".opengl";
			else if (Engine::Get().GetAPI() == RenderAPI::Vulkan)
				prefix = ".vulkan";
			else
			{
				KTN_CORE_ERROR("Unsupported API!")
				return "";
			}

			switch (p_Type)
			{
				case ShaderType::Vertex:    		return prefix + ".cached.vert";
				case ShaderType::Fragment:  		return prefix + ".cached.frag";
				case ShaderType::Geometry:  		return prefix + ".cached.geom";
				case ShaderType::TessControl:  		return prefix + ".cached.tesc";
				case ShaderType::TessEvaluation:	return prefix + ".cached.tese";
				case ShaderType::Compute:  			return prefix + ".cached.comp";
				default:
					KTN_CORE_ERROR("Unknown shader type!")
					return "";
			}
		}

		static std::vector<uint32_t> CompileGLSLToSPIRV(const std::string& p_Source, ShaderType p_Type)
		{
			KTN_PROFILE_FUNCTION();

			glslang::InitializeProcess();

			EShLanguage stage = ShaderTypeToGlslang(p_Type);
			glslang::TShader shader(stage);
			const char* shaderStrings[1];
			shaderStrings[0] = p_Source.c_str();
			shader.setStrings(shaderStrings, 1);

			std::string preamble =
				"#extension GL_ARB_shading_language_420pack : enable\n"
				"#extension GL_EXT_scalar_block_layout : enable\n";

			if (Engine::Get().GetAPI() == RenderAPI::OpenGL)
				preamble += "#define IS_OPENGL 1\n";
			else
				preamble += "#define IS_OPENGL 0\n";

			if (Engine::Get().GetAPI() == RenderAPI::Vulkan)
				preamble += "#define IS_VULKAN 1\n";
			else
				preamble += "#define IS_VULKAN 0\n";

			shader.setPreamble(preamble.c_str());

			int ClientInputSemanticsVersion					= 130;
			glslang::EShTargetClientVersion ClientVersion	= glslang::EShTargetVulkan_1_3;
			glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_5;
			glslang::EShClient Client						= glslang::EShClientVulkan;
			if (Engine::Get().GetAPI() == RenderAPI::OpenGL)
			{
				ClientInputSemanticsVersion					= 450;
				Client										= glslang::EShClientOpenGL;
				ClientVersion								= glslang::EShTargetOpenGL_450;
			}

			shader.setEnvInput(glslang::EShSourceGlsl, stage, Client, ClientInputSemanticsVersion);
			shader.setEnvClient(Client, ClientVersion);
			shader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);

			TBuiltInResource Resources						= DefaultTBuiltInResource;
			Resources.limits.generalUniformIndexing			= true;
			Resources.limits.generalVariableIndexing		= true;
			Resources.limits.generalSamplerIndexing			= true;
			Resources.limits.whileLoops						= true;
			Resources.limits.nonInductiveForLoops			= true;

			if (auto messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
				!shader.parse(&Resources, ClientInputSemanticsVersion, false, messages))
			{
				KTN_CORE_ERROR("GLSL Parsing Failed for shader: ");
				KTN_CORE_ERROR("{}", shader.getInfoLog());
				KTN_CORE_ERROR("{}", shader.getInfoDebugLog());
				KTN_CORE_ASSERT(false);
			}

			glslang::TProgram program;
			program.addShader(&shader);

			if (auto messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
				!program.link(messages))
			{
				KTN_CORE_ERROR("Program Linking Failed: ");
				KTN_CORE_ERROR("{}", program.getInfoLog());
				KTN_CORE_ERROR("{}", program.getInfoDebugLog());
				KTN_CORE_ASSERT(false);
			}

			std::vector<uint32_t> spirv;
			glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);

			glslang::FinalizeProcess();
			return spirv;
		}

	} // namespace 

	Ref<Shader> Shader::Create(const std::string& p_Path)
	{
		KTN_PROFILE_FUNCTION();

		if (Engine::Get().GetAPI() == RenderAPI::OpenGL)
			return CreateRef<GLShader>(CompileOrGetSpirv(p_Path));

		KTN_CORE_ERROR("Unsupported API!");
		return nullptr;
	}

	std::string Shader::ProcessIncludeFiles(const std::string& p_Path, const std::string& p_Code)
	{
		KTN_PROFILE_FUNCTION();

		auto result = std::string();

		const char* includeToken = "#include";
		size_t pos = p_Code.find(includeToken, 0);
		if (pos == std::string::npos)
		{
			return p_Code;
		}

		while (pos != std::string::npos)
		{
			if (result == std::string())
			{
				result = p_Code.substr(0, pos);
			}

			size_t eol = p_Code.find_first_of("\r\n", pos);
			size_t start = p_Code.find("<", pos);
			size_t end = p_Code.find(">", start);

			if (start == std::string::npos)
			{
				start = p_Code.find("\"", pos);
				end = p_Code.find("\"", start);
			}

			KTN_CORE_ASSERT(start != std::string::npos && end != std::string::npos,"Invalid include directive!");
			KTN_CORE_ASSERT(start < eol && end < eol, "Not on the same line!");

			std::string includeFilepath = p_Code.substr(start + 1, end - start - 1);
			std::filesystem::path filepath = p_Path;
			auto path = (filepath.parent_path() / includeFilepath).string();
			std::string includeCode = FileSystem::ReadFile(path);

			result += includeCode;

			pos = p_Code.find(includeToken, end + 1);
			if (pos == std::string::npos)
			{
				result += p_Code.substr(end + 1);
			}
			else
			{
				result += p_Code.substr(end + 1, pos - end - 1);
			}
		}

		return result;
	}

	ShaderSource Shader::Process(const std::string& p_Path)
	{
		KTN_PROFILE_FUNCTION();

		ShaderSource shaderSources;

		auto source = FileSystem::ReadFile(p_Path);


		std::string extension = FileSystem::GetExtension(p_Path);
		if (extension == ".comp")
		{
			shaderSources[ShaderType::Compute] = ProcessIncludeFiles(p_Path, source);
			return shaderSources;
		}

		const char* typeToken = "@type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos);
			KTN_CORE_ASSERT(eol != std::string::npos, "Syntax error!");

			size_t begin = source.find_first_not_of(" \t", pos + typeTokenLength);
			size_t end = source.find_last_not_of(" \t", eol);
			std::string type = source.substr(begin, end - begin + 1);
			if (size_t typeEnd = type.find_first_of(" \r\n");
				typeEnd != std::string::npos) 
			{
				type.erase(typeEnd);
			}

			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);
			auto code = source.substr(
				nextLinePos,
				pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos)
			);
			shaderSources[ShaderTypeFromString(type)] = ProcessIncludeFiles(p_Path, code);
		}

		return shaderSources;
	}

	SpirvSource Shader::CompileOrGetSpirv(const std::string& p_Path)
	{
		KTN_PROFILE_FUNCTION();

		auto shaderSource = Process(p_Path);

		FileSystem::CreateDirectories(s_CacheDirectory);

		std::filesystem::path cacheDirectory = s_CacheDirectory;

		std::unordered_map<ShaderType, std::vector<uint32_t>> spirv = {};
		for (auto&& [stage, source] : shaderSource)
		{
			std::string file = FileSystem::GetStem(p_Path) + ShaderTypeCachedFileExtension(stage);
			auto cachedPath = cacheDirectory / file;
			auto cachedPath2 = (std::filesystem::path)FileSystem::GetParent(p_Path) / file;

			std::ifstream in( cachedPath, std::ios::in | std::ios::binary );
			if (!in.is_open())
				in = std::ifstream( cachedPath2, std::ios::in | std::ios::binary );

			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = spirv[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				spirv[stage] = CompileGLSLToSPIRV(source, stage);

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = spirv[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}

		return spirv;
	}

} // namespace KTN
