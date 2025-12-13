#pragma once
#include "Koten/Core/Base.h"
#include "AABB.h"
#include "Koten/Core/UUID.h"


namespace KTN
{
	inline float AABBSurfaceArea(const AABB& p_AABB)
	{
		return p_AABB.Perimeter();
	}

	class KTN_API AABBTree
	{
		using NodeIndex = int32_t;

		public:

            static constexpr inline int32_t NullNode = -1;

            struct Node
            {
                bool IsLeaf() const
                {
                    return Child1 == NullNode;
                }

                AABB Aabb;

                NodeIndex Parent;
                NodeIndex Child1;
                NodeIndex Child2;

                NodeIndex Next;
                bool Moved;

                UUID EnttUUID = 0;
            };

            AABBTree();
            ~AABBTree();
            AABBTree(const AABBTree&) = delete;
            AABBTree& operator=(const AABBTree&) = delete;

            void Reset();

            NodeIndex CreateNode(UUID p_UUID, const AABB& p_AABB);
            bool MoveNode(NodeIndex p_Node, AABB p_AABB, const glm::vec2& p_Displacement, bool p_ForceMove);
            void RemoveNode(NodeIndex p_Node);

            bool TestOverlap(NodeIndex p_NodeA, NodeIndex p_NodeB) const;
            const AABB& GetAABB(NodeIndex p_Node) const;
            void ClearMoved(NodeIndex p_Node) const;
            bool WasMoved(NodeIndex p_Node) const;
            UUID GetUUID(NodeIndex p_Node) const;

            void Traverse(std::function<void(const Node*)> p_Callback) const;
            void Query(const glm::vec2& p_Point, std::function<bool(NodeIndex, UUID)> p_Callback) const;
            void Query(const AABB& p_AABB, std::function<bool(NodeIndex, UUID)> p_Callback) const;
            std::vector<NodeIndex> QueryRegion(const AABB& p_Region) const;

            std::vector<std::pair<UUID, UUID>> ComputePairs() const;
            float ComputeTreeCost() const;
            void Rebuild();

		private:
            NodeIndex AllocateNode();
            void FreeNode(NodeIndex p_Node);

            NodeIndex InsertLeaf(NodeIndex p_Leaf);
            void RemoveLeaf(NodeIndex p_Leaf);

            void Rotate(NodeIndex p_Node);
            void Swap(NodeIndex p_Node1, NodeIndex p_Node2);

        private:
            NodeIndex m_Root;

            Node* m_Nodes;
            int32_t m_NodeCapacity;
            int32_t m_NodeCount;

            NodeIndex m_FreeList;
	};

} // namespace KTN