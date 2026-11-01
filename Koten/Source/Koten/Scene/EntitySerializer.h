#pragma once
#include "Koten/Core/Buffer.h"
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
            static bool DeserializeBin(std::ifstream& p_In, Buffer& p_Buffer);
            static bool DeserializeBin(BufferReader& p_In, Entity& p_Entt);

            static void ClearFunctions();
            static void ExecuteFunctions();

            static void PushFunction(const std::function<void()>& p_Func) { m_Functions.push_back(p_Func); }
            static std::vector<std::function<void()>>& GetFunctions() { return m_Functions; }

        private:
            inline static std::vector<std::function<void()>> m_Functions;
    };

} // namespace KTN
