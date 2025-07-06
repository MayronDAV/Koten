#include "ktnpch.h"

// lib
#include <yaml-cpp/yaml.h>



namespace YAML
{
	Emitter& operator <<(Emitter& p_Out, const glm::vec2& p_Value)
	{
		p_Out << Flow;
		p_Out << BeginSeq << p_Value.x << p_Value.y << EndSeq;
		return p_Out;
	}

	Emitter& operator <<(Emitter& p_Out, const glm::vec3& p_Value)
	{
		p_Out << Flow;
		p_Out << BeginSeq << p_Value.x << p_Value.y << p_Value.z << EndSeq;
		return p_Out;
	}

	Emitter& operator <<(Emitter& p_Out, const glm::vec4& p_Value)
	{
		p_Out << Flow;
		p_Out << BeginSeq << p_Value.x << p_Value.y << p_Value.z << p_Value.w << EndSeq;
		return p_Out;
	}

	Emitter& operator<<(Emitter& p_Out, const KTN::AssetHandle& p_Handle)
	{
		p_Out << (uint64_t)p_Handle;
		return p_Out;
	}
} // namespace YAML