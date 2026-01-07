#pragma once

#include "Entity.h"


namespace YAML
{
	class Emitter;
	class Node;
}

namespace KTN
{
	class KTN_API EntitySerializer
	{
		public:
			static void Serialize(YAML::Emitter* p_Out, const Entity& p_Entt);
			static bool Deserialize(YAML::Node* p_Node, Entity& p_Entt);

			static void SerializeBin(std::ofstream& p_Out, const Entity& p_Entt);
			static bool DeserializeBin(std::ifstream& p_In, Entity& p_Entt);
	};

} // namespace KTN
