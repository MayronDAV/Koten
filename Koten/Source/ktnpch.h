#if defined(_MSC_VER)
#pragma once
#endif

#include "Koten/Core/Base.h"
#include "Koten/Core/Log.h"
#include "Koten/Core/Assert.h"
#include "Koten/Core/Engine.h"
#include "Koten/Core/FileSystem.h"
#include "Koten/Core/Time.h"
#include "Koten/Core/UUID.h"
#include "Koten/Utils/HashCombiner.h"
#include "Koten/Utils/Utils.h"
#include "Koten/Asset/Asset.h"

// lib
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// std
#include <istream>
#include <fstream>
#include <iostream>
#include <memory>
#include <algorithm>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <cstdint>
#ifdef KTN_WINDOWS
	#include <Windows.h>
#endif


#ifdef KTN_EXPORT

// lib
#include <yaml-cpp/yaml.h>

namespace YAML
{
	template<>
	struct convert<KTN::UUID>
	{
		static Node encode(const KTN::UUID& p_UUID)
		{
			Node node;
			node.push_back((uint64_t)p_UUID);
			return node;
		}

		static bool decode(const Node& p_Node, KTN::UUID& p_UUID)
		{
			p_UUID = p_Node.as<uint64_t>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& p_Rhs)
		{
			Node node;
			node.push_back(p_Rhs.x);
			node.push_back(p_Rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& p_Node, glm::vec2& p_Rhs)
		{
			if (!p_Node.IsSequence() || p_Node.size() != 2)
				return false;

			p_Rhs.x = p_Node[0].as<float>();
			p_Rhs.y = p_Node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& p_Rhs)
		{
			Node node;
			node.push_back(p_Rhs.x);
			node.push_back(p_Rhs.y);
			node.push_back(p_Rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& p_Node, glm::vec3& p_Rhs)
		{
			if (!p_Node.IsSequence() || p_Node.size() != 3)
				return false;

			p_Rhs.x = p_Node[0].as<float>();
			p_Rhs.y = p_Node[1].as<float>();
			p_Rhs.z = p_Node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& p_Rhs)
		{
			Node node;
			node.push_back(p_Rhs.x);
			node.push_back(p_Rhs.y);
			node.push_back(p_Rhs.z);
			node.push_back(p_Rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& p_Node, glm::vec4& p_Rhs)
		{
			if (!p_Node.IsSequence() || p_Node.size() != 4)
				return false;

			p_Rhs.x = p_Node[0].as<float>();
			p_Rhs.y = p_Node[1].as<float>();
			p_Rhs.z = p_Node[2].as<float>();
			p_Rhs.w = p_Node[3].as<float>();
			return true;
		}
	};

	inline Emitter& operator <<(Emitter& p_Out, const glm::vec2& p_Value)
	{
		p_Out << Flow;
		p_Out << BeginSeq << p_Value.x << p_Value.y << EndSeq;
		return p_Out;
	}

	inline Emitter& operator <<(Emitter& p_Out, const glm::vec3& p_Value)
	{
		p_Out << Flow;
		p_Out << BeginSeq << p_Value.x << p_Value.y << p_Value.z << EndSeq;
		return p_Out;
	}

	inline Emitter& operator <<(Emitter& p_Out, const glm::vec4& p_Value)
	{
		p_Out << Flow;
		p_Out << BeginSeq << p_Value.x << p_Value.y << p_Value.z << p_Value.w << EndSeq;
		return p_Out;
	}

	inline Emitter& operator<<(Emitter& p_Out, const KTN::AssetHandle& p_Handle)
	{
		p_Out << (uint64_t)p_Handle;
		return p_Out;
	}

} // namespace YAML

#endif