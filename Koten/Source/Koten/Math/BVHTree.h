#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Core/Definitions.h"
#include "BoundingBox.h"
#include "Koten/Core/JobGroup.h"

// lib
#include <glm/glm.hpp>

// std
#include <vector>
#include <unordered_map>
#include <mutex>
#include <functional>



namespace KTN::Math
{
    using BVHObjectID     = uint64_t;
    using BVHIntersectsFn = std::function<Intersection(const BoundingBox&)>;
    // if it's true, the callback will be called only once
    using BVHCallbackFn   = std::function<bool(BVHObjectID, const BoundingBox&)>;
    using BVHInsertFn     = std::function<std::pair<BVHObjectID, Math::BoundingBox>()>;

    inline constexpr BVHObjectID INVALID_BVH_ID = UINT64_MAX;

    struct BVHSettings
    {
        uint32_t MaxObjectsPerLeaf    = 8;
        uint32_t MaxDepth             = 16;

        float FatBoundsMargin         = 0.3f;
        float FatBoundsRelativeMargin = 0.3f;
    };

    class KTN_API BVHTree
    {
        struct Object
        {
            BVHObjectID ID = INVALID_BVH_ID;
            BoundingBox Bounds;
            BoundingBox FatBounds;

            bool IsValid() const { return ID != INVALID_BVH_ID && Bounds.IsValid(); }
        };

        struct Node
        {
            BoundingBox Bounds;

            Node* Parent = nullptr;

            Unique<Node> Left;
            Unique<Node> Right;

            std::vector<Object> Objects;

            bool IsLeaf() const
            {
                return !Left && !Right;
            }
        };

        public:
            explicit BVHTree(const BVHSettings& p_Settings = {});
            ~BVHTree() = default;

            void SetSettings(const BVHSettings& p_Settings);

            bool Insert(BVHObjectID p_ID, const BoundingBox& p_Bounds);
            bool Update(BVHObjectID p_ID, const BoundingBox& p_Bounds);
            void Query(const BVHIntersectsFn& p_Intersects, const BVHCallbackFn& p_Callback);

            void Clear();
            bool Remove(BVHObjectID p_ID);

            bool Contains(BVHObjectID p_ID) const;
            size_t GetObjectCount() const;
            BVHSettings GetSettings() const;

        private:
            BoundingBox CreateFatBounds(const BoundingBox& p_Bounds) const;
            bool RemoveInternal(BVHObjectID p_ID);
            bool InsertInto(Node* p_Node, const Object& p_Object, uint32_t p_Depth);
            Node* ChooseBestChild(Node* p_Node, const BoundingBox& p_Bounds) const;
            bool Split(Node* p_Node, uint32_t p_Depth);
            bool Refit(Node* p_Node);
            bool RefitUpwards(Node* p_Node);
            bool TryMergeUpwards(Node* p_Node);
            void QueryNode(const Node* p_Node, const BVHIntersectsFn& p_Intersects, const BVHCallbackFn& p_Callback, bool p_ParentInside = false);
            void QueryNodeAsync(const Node* p_Node, const BVHIntersectsFn& p_Intersects, const BVHCallbackFn& p_Callback, bool p_ParentInside = false);

        private:
            mutable std::mutex m_Mutex;
            std::mutex m_CallbackMutex;

            Unique<Node> m_Root;
            std::unordered_map<BVHObjectID, Node*> m_ObjectLeaves;

            BVHSettings m_Settings = {};
    };

} // namespace KTN::Math
