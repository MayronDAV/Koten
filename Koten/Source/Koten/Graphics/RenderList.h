#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Graphics/Texture.h"
#include "Koten/Graphics/DFFont.h"
#include "Koten/Math/BoundingSphere.h"
#include "Koten/Math/BoundingBox.h"

// lib
#include <glm/glm.hpp>

// std
#include <vector>
#include <variant>



namespace KTN
{
    using PickingID = uint32_t;
    inline constexpr PickingID INVALID_PICKING_ID = 0;

    struct SpriteCommand
    {
        Ref<Texture2D> Texture = nullptr;
        glm::vec4 Color        = { 1.0f, 1.0f, 1.0f, 1.0f };
        RenderType2D Type      = RenderType2D::Quad;

        // Circle

        float Thickness        = 1.0f;
        float Fade             = 0.005f; // 0.0f = no fade, 1.0f = full fade

        // UV Options

        bool UseDirectUVs      = false;
        glm::vec4 UV           = { 0.0f, 0.0f, 1.0f, 1.0f };

        glm::vec2 Size         = { 0.0f, 0.0f };
        // [true] if you want to pass the tile coord as a multiplier of the tile size
        // [false] if you want to pass the actual coord directly
        bool BySize            = true;
        glm::vec2 Offset       = { 0.0f, 0.0f };
        glm::vec2 Scale        = { 1.0f, 1.0f };
    };

    struct LineCommand
    {
        bool Primitive       = true;
        float Width          = 1.0f;
        glm::vec4 Color      = { 1.0f, 1.0f, 1.0f, 1.0f };
        glm::vec3 Start      = { 0.0f, 0.0f, 0.0f };
        glm::vec3 End        = { 1.0f, 0.0f, 0.0f };

        LineCommand()        = default;
        ~LineCommand()       = default;
    };

    struct TextCommand
    {
        Ref<DFFont> Font      = nullptr;
        std::string Text      = "";

        glm::vec4 Color       = { 1.0f, 1.0f, 1.0f, 1.0f };
        glm::vec4 BgColor     = { 0.0f, 0.0f, 0.0f, 0.0f };
        glm::vec4 CharBgColor = { 0.0f, 0.0f, 0.0f, 0.0f };

        bool DrawBg           = false;
        float LineSpacing     = 0.0f;
        float Kerning         = 0.0f;
    };

    struct RenderCommand
    {
        PickingID ID          = INVALID_PICKING_ID;
        glm::mat4 Transform   = { 1.0f };

        std::variant<std::monostate, Math::BoundingBox, Math::BoundingSphere> Bounds;

        std::variant<LineCommand, TextCommand, SpriteCommand> Command;
    };

    class KTN_API RenderList
    {
        public:
            RenderList()  = default;
            RenderList(const RenderList& p_List) : m_Commands(p_List.m_Commands) {}
            RenderList(const std::vector<RenderCommand>& p_Commands) : m_Commands(p_Commands) {}
            ~RenderList() = default;

            void Clear() { m_Commands.clear(); }

            bool Empty() const { return m_Commands.empty(); }

            void Submit(const RenderCommand& Command) { m_Commands.push_back(Command); }

            const std::vector<RenderCommand>& GetCommands() const { return m_Commands; }

            size_t Size() const { return m_Commands.size(); }

            RenderCommand& operator[](size_t p_Index) { return m_Commands[p_Index]; }
            const RenderCommand& operator[](size_t p_Index) const { return m_Commands[p_Index]; }

            RenderCommand& back() { return m_Commands.back(); }

            std::vector<RenderCommand>::iterator begin()          { return m_Commands.begin(); }
            std::vector<RenderCommand>::iterator end()            { return m_Commands.end(); }
            std::vector<RenderCommand>::reverse_iterator rbegin() { return m_Commands.rbegin(); }
            std::vector<RenderCommand>::reverse_iterator rend()   { return m_Commands.rend(); }

            std::vector<RenderCommand>::const_iterator begin()          const { return m_Commands.begin(); }
            std::vector<RenderCommand>::const_iterator end()            const { return m_Commands.end(); }
            std::vector<RenderCommand>::const_reverse_iterator rbegin() const { return m_Commands.rbegin(); }
            std::vector<RenderCommand>::const_reverse_iterator rend()   const { return m_Commands.rend(); }

        private:
            std::vector<RenderCommand> m_Commands;
    };

} // namespace KTN