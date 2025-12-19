#include "ktnpch.h"
#include "AABBTree.h"


namespace KTN
{
	static constexpr float s_Margin = 0.03f;
	static constexpr float s_Multiplier = 3.0f;

	AABBTree::AABBTree()
		: m_Root{ NullNode }
		, m_NodeCapacity{ 32 }
		, m_NodeCount{ 0 }
	{
		m_Nodes = (Node*)malloc(m_NodeCapacity * sizeof(Node));
		memset(m_Nodes, 0, m_NodeCapacity * sizeof(Node));

		for (int32_t i = 0; i < m_NodeCapacity - 1; ++i)
		{
			m_Nodes[i].Next = i + 1;
			m_Nodes[i].Parent = i;
		}
		m_Nodes[m_NodeCapacity - 1].Next = NullNode;
		m_Nodes[m_NodeCapacity - 1].Parent = m_NodeCapacity - 1;

		m_FreeList = 0;
	}

	AABBTree::~AABBTree()
	{
		free(m_Nodes);
		m_Root = NullNode;
		m_NodeCount = 0;
	}

	void AABBTree::Reset()
	{
		KTN_PROFILE_FUNCTION();

		m_Root = NullNode;
		m_NodeCount = 0;
		memset(m_Nodes, 0, m_NodeCapacity * sizeof(Node));

		for (int32_t i = 0; i < m_NodeCapacity - 1; ++i)
		{
			m_Nodes[i].Next = i + 1;
			m_Nodes[i].Parent = i;
		}
		m_Nodes[m_NodeCapacity - 1].Next = NullNode;
		m_Nodes[m_NodeCapacity - 1].Parent = m_NodeCapacity - 1;

		m_FreeList = 0;
	}

	AABBTree::NodeIndex AABBTree::CreateNode(UUID p_UUID, const AABB& p_AABB)
	{
		KTN_PROFILE_FUNCTION();

		NodeIndex newNode = AllocateNode();
		AABB aabb = p_AABB;
		aabb.Expand(s_Margin);

		m_Nodes[newNode].Aabb = aabb;
		m_Nodes[newNode].EnttUUID = p_UUID;
		m_Nodes[newNode].Parent = NullNode;
		m_Nodes[newNode].Moved = true;

		InsertLeaf(newNode);

		return newNode;
	}

	bool AABBTree::MoveNode(NodeIndex p_Node, AABB p_AABB, const glm::vec2& p_Displacement, bool p_ForceMove)
	{
		KTN_PROFILE_FUNCTION();

		KTN_CORE_ASSERT(0 <= p_Node && p_Node < m_NodeCapacity);
		KTN_CORE_ASSERT(m_Nodes[p_Node].IsLeaf());

		const AABB& treeAABB = m_Nodes[p_Node].Aabb;
		if (treeAABB.Contains(p_AABB) && p_ForceMove == false)
			return false;

		glm::vec2 d = p_Displacement * s_Multiplier;

		if (d.x > 0.0f)
			p_AABB.Max.x += d.x;
		else
			p_AABB.Min.x += d.x;

		if (d.y > 0.0f)
			p_AABB.Max.y += d.y;
		else
			p_AABB.Min.y += d.y;

		p_AABB.Expand(s_Margin);

		RemoveLeaf(p_Node);

		m_Nodes[p_Node].Aabb = p_AABB;

		InsertLeaf(p_Node);

		m_Nodes[p_Node].Moved = true;

		return true;
	}

	void AABBTree::RemoveNode(NodeIndex p_Node)
	{
		KTN_PROFILE_FUNCTION();

		KTN_CORE_ASSERT(0 <= p_Node && p_Node < m_NodeCapacity);
		KTN_CORE_ASSERT(m_Nodes[p_Node].IsLeaf());

		RemoveLeaf(p_Node);
		FreeNode(p_Node);
	}

	bool AABBTree::TestOverlap(NodeIndex p_NodeA, NodeIndex p_NodeB) const
	{
		KTN_PROFILE_FUNCTION();

		KTN_CORE_ASSERT(0 <= p_NodeA && p_NodeA < m_NodeCapacity);
		KTN_CORE_ASSERT(0 <= p_NodeB && p_NodeB < m_NodeCapacity);

		return m_Nodes[p_NodeA].Aabb.Overlaps(m_Nodes[p_NodeB].Aabb);
	}

	const AABB& AABBTree::GetAABB(NodeIndex p_Node) const
	{
		KTN_CORE_ASSERT(0 <= p_Node && p_Node < m_NodeCapacity);

		return m_Nodes[p_Node].Aabb;
	}

	void AABBTree::ClearMoved(NodeIndex p_Node) const
	{
		KTN_CORE_ASSERT(0 <= p_Node && p_Node < m_NodeCapacity);

		m_Nodes[p_Node].Moved = false;
	}

	bool AABBTree::WasMoved(NodeIndex p_Node) const
	{
		KTN_CORE_ASSERT(0 <= p_Node && p_Node < m_NodeCapacity);

		return m_Nodes[p_Node].Moved;
	}

	UUID AABBTree::GetUUID(NodeIndex p_Node) const
	{
		KTN_CORE_ASSERT(0 <= p_Node && p_Node < m_NodeCapacity);

		return m_Nodes[p_Node].EnttUUID;
	}

	void AABBTree::Traverse(std::function<void(const Node*)> p_Callback) const
	{
		KTN_PROFILE_FUNCTION();

		if (m_Root == NullNode)
			return;

		std::vector<NodeIndex> stack;
		stack.reserve(64);
		stack.emplace_back(m_Root);

		while (stack.size() != 0)
		{
			NodeIndex current = stack.back();
			stack.pop_back();

			if (!m_Nodes[current].IsLeaf())
			{
				stack.emplace_back(m_Nodes[current].Child1);
				stack.emplace_back(m_Nodes[current].Child2);
			}

			const Node* node = m_Nodes + current;
			p_Callback(node);
		}
	}

	void AABBTree::Query(const glm::vec2& p_Point, std::function<bool(NodeIndex, UUID)> p_Callback) const
	{
		KTN_PROFILE_FUNCTION();

		if (m_Root == NullNode)
			return;

		std::vector<NodeIndex> stack;
		stack.reserve(64);
		stack.emplace_back(m_Root);

		while (stack.size() != 0)
		{
			NodeIndex current = stack.back();
			stack.pop_back();

			if (!m_Nodes[current].Aabb.Overlaps(p_Point))
				continue;

			if (m_Nodes[current].IsLeaf())
			{
				bool proceed = p_Callback(current, m_Nodes[current].EnttUUID);
				if (!proceed)
					return;
			}
			else
			{
				stack.emplace_back(m_Nodes[current].Child1);
				stack.emplace_back(m_Nodes[current].Child2);
			}
		}
	}

	void AABBTree::Query(const AABB& p_AABB, std::function<bool(NodeIndex, UUID)> p_Callback) const
	{
		KTN_PROFILE_FUNCTION();

		if (m_Root == NullNode)
			return;

		std::vector<NodeIndex> stack;
		stack.reserve(64);
		stack.emplace_back(m_Root);

		while (stack.size() != 0)
		{
			NodeIndex current = stack.back();
			stack.pop_back();

			if (!m_Nodes[current].Aabb.Overlaps(p_AABB))
				continue;

			if (m_Nodes[current].IsLeaf())
			{
				bool proceed = p_Callback(current, m_Nodes[current].EnttUUID);
				if (!proceed)
					return;
			}
			else
			{
				stack.emplace_back(m_Nodes[current].Child1);
				stack.emplace_back(m_Nodes[current].Child2);
			}
		}
	}

	std::vector<AABBTree::NodeIndex> AABBTree::QueryRegion(const AABB& p_Region) const
	{
		KTN_PROFILE_FUNCTION();

		std::vector<NodeIndex> result;

		if (m_Root == NullNode)
			return result;

		std::vector<NodeIndex> stack;
		stack.reserve(64);
		stack.emplace_back(m_Root);


		while (stack.size() != 0)
		{
			NodeIndex current = stack.back();
			stack.pop_back();

			if (!m_Nodes[current].Aabb.Overlaps(p_Region))
				continue;

			if (m_Nodes[current].IsLeaf())
				result.push_back(current);
			else
			{
				stack.emplace_back(m_Nodes[current].Child1);
				stack.emplace_back(m_Nodes[current].Child2);
			}
		}

		return result;
	}

	std::vector<std::pair<UUID, UUID>> AABBTree::ComputePairs() const
	{
		KTN_PROFILE_FUNCTION();

		std::vector<std::pair<UUID, UUID>> pairs;
		pairs.reserve(128);

		std::vector<NodeIndex> leaves;
		leaves.reserve(m_NodeCount);

		for (NodeIndex i = 0; i < m_NodeCapacity; ++i)
		{
			Node& node = m_Nodes[i];
			if (node.Parent == i) continue;
			if (!node.IsLeaf()) continue;

			leaves.push_back(i);
		}

		for (size_t i = 0; i < leaves.size(); ++i)
		{
			NodeIndex leafA = leaves[i];
			const Node& nodeA = m_Nodes[leafA];
			const UUID& idA = nodeA.EnttUUID;
			const AABB& aabbA = nodeA.Aabb;

			for (size_t j = i + 1; j < leaves.size(); ++j)
			{
				NodeIndex leafB = leaves[j];
				const Node& nodeB = m_Nodes[leafB];

				if (!aabbA.Overlaps(nodeB.Aabb))
					continue;

				const UUID& idB = nodeB.EnttUUID;

				UUID a = idA;
				UUID b = idB;
				if (a > b)
					std::swap(a, b);

				pairs.emplace_back(a, b);
			}
		}

		for (NodeIndex i = 0; i < m_NodeCapacity; ++i)
		{
			if (m_Nodes[i].Parent != i)
			{
				m_Nodes[i].Moved = false;
			}
		}

		return pairs;
	}

	float AABBTree::ComputeTreeCost() const
	{
		KTN_PROFILE_FUNCTION();

		float cost = 0.0f;

		Traverse([&cost](const Node* p_Node) -> void { cost += AABBSurfaceArea(p_Node->Aabb); });

		return cost;
	}

	void AABBTree::Rebuild()
	{
		KTN_PROFILE_FUNCTION();

		NodeIndex* leaves = (NodeIndex*)malloc(m_NodeCount * sizeof(NodeIndex));
		int32_t count = 0;

		for (int32_t i = 0; i < m_NodeCapacity; ++i)
		{
			if (m_Nodes[i].Parent == i)
				continue;

			if (m_Nodes[i].IsLeaf())
			{
				m_Nodes[i].Parent = NullNode;
				leaves[count++] = i;
			}
			else
				FreeNode(i);
		}

		while (count > 1)
		{
			float minCost = FLT_MAX;
			int32_t minI = -1;
			int32_t minJ = -1;

			for (int32_t i = 0; i < count; ++i)
			{
				AABB aabbI = m_Nodes[leaves[i]].Aabb;

				for (int32_t j = i + 1; j < count; ++j)
				{
					AABB aabbJ = m_Nodes[leaves[j]].Aabb;

					AABB combined = AABB::Union(aabbI, aabbJ);
					float cost = AABBSurfaceArea(combined);

					if (cost < minCost)
					{
						minCost = cost;
						minI = i;
						minJ = j;
					}
				}
			}

			NodeIndex index1 = leaves[minI];
			NodeIndex index2 = leaves[minJ];
			Node* child1 = m_Nodes + index1;
			Node* child2 = m_Nodes + index2;

			NodeIndex parentIndex = AllocateNode();
			Node* parent = m_Nodes + parentIndex;

			parent->Child1 = index1;
			parent->Child2 = index2;
			parent->Aabb = AABB::Union(child1->Aabb, child2->Aabb);
			parent->Parent = NullNode;

			child1->Parent = parentIndex;
			child2->Parent = parentIndex;

			leaves[minI] = parentIndex;

			leaves[minJ] = leaves[count - 1];
			--count;
		}

		m_Root = leaves[0];
		free(leaves);
	}

	AABBTree::NodeIndex AABBTree::AllocateNode()
	{
		KTN_PROFILE_FUNCTION();

		if (m_FreeList == NullNode)
		{
			KTN_CORE_ASSERT(m_NodeCount == m_NodeCapacity);

			Node* oldNodes = m_Nodes;
			int32_t oldCapacity = m_NodeCapacity;
			m_NodeCapacity += m_NodeCapacity / 2;
			m_Nodes = (Node*)malloc(m_NodeCapacity * sizeof(Node));
			memcpy(m_Nodes, oldNodes, oldCapacity * sizeof(Node));
			memset(m_Nodes + oldCapacity, 0, (m_NodeCapacity - oldCapacity) * sizeof(Node));
			free(oldNodes);

			for (int32_t i = oldCapacity; i < m_NodeCapacity - 1; ++i)
			{
				m_Nodes[i].Next = i + 1;
				m_Nodes[i].Parent = i;
			}
			m_Nodes[m_NodeCapacity - 1].Next = NullNode;
			m_Nodes[m_NodeCapacity - 1].Parent = m_NodeCapacity - 1;

			m_FreeList = oldCapacity;

		}

		NodeIndex node = m_FreeList;
		m_FreeList = m_Nodes[node].Next;
		m_Nodes[node].Parent = NullNode;
		m_Nodes[node].Child1 = NullNode;
		m_Nodes[node].Child2 = NullNode;
		m_Nodes[node].Moved = false;
		++m_NodeCount;

		return node;
	}

	void AABBTree::FreeNode(NodeIndex p_Node)
	{
		KTN_PROFILE_FUNCTION();

		KTN_CORE_ASSERT(0 <= p_Node && p_Node <= m_NodeCapacity);
		KTN_CORE_ASSERT(0 < m_NodeCount);

		m_Nodes[p_Node].Parent = p_Node;
		m_Nodes[p_Node].Next = m_FreeList;
		m_FreeList = p_Node;

		--m_NodeCount;
	}

	AABBTree::NodeIndex AABBTree::InsertLeaf(NodeIndex p_Leaf)
	{
		KTN_PROFILE_FUNCTION();

		KTN_CORE_ASSERT(0 <= p_Leaf && p_Leaf < m_NodeCapacity);
		KTN_CORE_ASSERT(m_Nodes[p_Leaf].IsLeaf());

		if (m_Root == NullNode)
		{
			m_Root = p_Leaf;
			return p_Leaf;
		}

		AABB aabb = m_Nodes[p_Leaf].Aabb;

		NodeIndex bestSibling = m_Root;
		float bestCost = AABBSurfaceArea(AABB::Union(m_Nodes[m_Root].Aabb, aabb));

		struct Candidate
		{
			NodeIndex node;
			float inheritedCost;
		};

		std::vector<Candidate> stack;
		stack.reserve(256);
		stack.emplace_back(m_Root, 0.0f);

		while (stack.size() != 0)
		{
			NodeIndex current = stack.back().node;
			float inheritedCost = stack.back().inheritedCost;
			stack.pop_back();

			AABB combined = AABB::Union(m_Nodes[current].Aabb, aabb);
			float directCost = AABBSurfaceArea(combined);

			float cost = directCost + inheritedCost;
			if (cost < bestCost)
			{
				bestCost = cost;
				bestSibling = current;
			}

			inheritedCost += directCost - AABBSurfaceArea(m_Nodes[current].Aabb);

			float lowerBoundCost = AABBSurfaceArea(aabb) + inheritedCost;
			if (lowerBoundCost < bestCost)
			{
				if (m_Nodes[current].IsLeaf() == false)
				{
					stack.emplace_back(m_Nodes[current].Child1, inheritedCost);
					stack.emplace_back(m_Nodes[current].Child2, inheritedCost);
				}
			}
		}

		NodeIndex oldParent = m_Nodes[bestSibling].Parent;
		NodeIndex newParent = AllocateNode();
		m_Nodes[newParent].Aabb = AABB::Union(aabb, m_Nodes[bestSibling].Aabb);
		m_Nodes[newParent].EnttUUID = 0;
		m_Nodes[newParent].Parent = oldParent;

		m_Nodes[newParent].Child1 = p_Leaf;
		m_Nodes[newParent].Child2 = bestSibling;
		m_Nodes[p_Leaf].Parent = newParent;
		m_Nodes[bestSibling].Parent = newParent;

		if (oldParent != NullNode)
		{
			if (m_Nodes[oldParent].Child1 == bestSibling)
				m_Nodes[oldParent].Child1 = newParent;
			else
				m_Nodes[oldParent].Child2 = newParent;
		}
		else
			m_Root = newParent;

		NodeIndex ancestor = newParent;
		while (ancestor != NullNode)
		{
			NodeIndex child1 = m_Nodes[ancestor].Child1;
			NodeIndex child2 = m_Nodes[ancestor].Child2;

			m_Nodes[ancestor].Aabb = AABB::Union(m_Nodes[child1].Aabb, m_Nodes[child2].Aabb);

			Rotate(ancestor);

			ancestor = m_Nodes[ancestor].Parent;
		}

		return p_Leaf;
	}

	void AABBTree::RemoveLeaf(NodeIndex p_Leaf)
	{
		KTN_PROFILE_FUNCTION();

		KTN_CORE_ASSERT(0 <= p_Leaf && p_Leaf < m_NodeCapacity);
		KTN_CORE_ASSERT(m_Nodes[p_Leaf].IsLeaf());

		NodeIndex parent = m_Nodes[p_Leaf].Parent;
		if (parent == NullNode)
		{
			KTN_CORE_ASSERT(m_Root == p_Leaf);
			m_Root = NullNode;
			return;
		}

		NodeIndex grandParent = m_Nodes[parent].Parent;
		NodeIndex sibling;
		if (m_Nodes[parent].Child1 == p_Leaf)
			sibling = m_Nodes[parent].Child2;
		else
			sibling = m_Nodes[parent].Child1;

		FreeNode(parent);

		if (grandParent != NullNode)
		{
			m_Nodes[sibling].Parent = grandParent;

			if (m_Nodes[grandParent].Child1 == parent)
				m_Nodes[grandParent].Child1 = sibling;
			else
				m_Nodes[grandParent].Child2 = sibling;

			NodeIndex ancestor = grandParent;
			while (ancestor != NullNode)
			{
				NodeIndex child1 = m_Nodes[ancestor].Child1;
				NodeIndex child2 = m_Nodes[ancestor].Child2;

				m_Nodes[ancestor].Aabb = AABB::Union(m_Nodes[child1].Aabb, m_Nodes[child2].Aabb);

				Rotate(ancestor);

				ancestor = m_Nodes[ancestor].Parent;
			}
		}
		else
		{
			m_Root = sibling;
			m_Nodes[sibling].Parent = NullNode;
		}
	}

	void AABBTree::Rotate(NodeIndex p_Node)
	{
		KTN_PROFILE_FUNCTION();

		if (m_Nodes[p_Node].IsLeaf())
			return;

		NodeIndex child1 = m_Nodes[p_Node].Child1;
		NodeIndex child2 = m_Nodes[p_Node].Child2;

		float costDiffs[4] = { 0.0f };

		if (m_Nodes[child1].IsLeaf() == false)
		{
			float area1 = AABBSurfaceArea(m_Nodes[child1].Aabb);
			costDiffs[0] = AABBSurfaceArea(AABB::Union(m_Nodes[m_Nodes[child1].Child1].Aabb, m_Nodes[child2].Aabb)) - area1;
			costDiffs[1] = AABBSurfaceArea(AABB::Union(m_Nodes[m_Nodes[child1].Child2].Aabb, m_Nodes[child2].Aabb)) - area1;
		}

		if (m_Nodes[child2].IsLeaf() == false)
		{
			float area2 = AABBSurfaceArea(m_Nodes[child2].Aabb);
			costDiffs[2] = AABBSurfaceArea(AABB::Union(m_Nodes[m_Nodes[child2].Child1].Aabb, m_Nodes[child1].Aabb)) - area2;
			costDiffs[3] = AABBSurfaceArea(AABB::Union(m_Nodes[m_Nodes[child2].Child2].Aabb, m_Nodes[child1].Aabb)) - area2;
		}

		int32_t bestDiffIndex = 0;
		for (int32_t i = 1; i < 4; ++i)
		{
			if (costDiffs[i] < costDiffs[bestDiffIndex])
				bestDiffIndex = i;
		}

		if (costDiffs[bestDiffIndex] >= 0.0f)
			return;

		switch (bestDiffIndex)
		{
			case 0:
			{
				// Swap(child2, m_Nodes[child1].Child2);
				m_Nodes[m_Nodes[child1].Child2].Parent = p_Node;
				m_Nodes[p_Node].Child2 = m_Nodes[child1].Child2;

				m_Nodes[child1].Child2 = child2;
				m_Nodes[child2].Parent = child1;

				m_Nodes[child1].Aabb = AABB::Union(m_Nodes[m_Nodes[child1].Child1].Aabb, m_Nodes[m_Nodes[child1].Child2].Aabb);
			}
			break;
			case 1:
			{
				// Swap(child2, m_Nodes[child1].Child1);
				m_Nodes[m_Nodes[child1].Child1].Parent = p_Node;
				m_Nodes[p_Node].Child2 = m_Nodes[child1].Child1;

				m_Nodes[child1].Child1 = child2;
				m_Nodes[child2].Parent = child1;

				m_Nodes[child1].Aabb = AABB::Union(m_Nodes[m_Nodes[child1].Child1].Aabb, m_Nodes[m_Nodes[child1].Child2].Aabb);
			}
			break;
			case 2:
			{
				// Swap(child1, m_Nodes[child2].Child2);
				m_Nodes[m_Nodes[child2].Child2].Parent = p_Node;
				m_Nodes[p_Node].Child1 = m_Nodes[child2].Child2;

				m_Nodes[child2].Child2 = child1;
				m_Nodes[child1].Parent = child2;

				m_Nodes[child2].Aabb = AABB::Union(m_Nodes[m_Nodes[child2].Child1].Aabb, m_Nodes[m_Nodes[child2].Child2].Aabb);
			}
			break;
			case 3:
			{
				// Swap(child1, m_Nodes[child2].Child1);
				m_Nodes[m_Nodes[child2].Child1].Parent = p_Node;
				m_Nodes[p_Node].Child1 = m_Nodes[child2].Child1;

				m_Nodes[child2].Child1 = child1;
				m_Nodes[child1].Parent = child2;

				m_Nodes[child2].Aabb = AABB::Union(m_Nodes[m_Nodes[child2].Child1].Aabb, m_Nodes[m_Nodes[child2].Child2].Aabb);
			}
			break;
		}
	}

	void AABBTree::Swap(NodeIndex p_Node1, NodeIndex p_Node2)
	{
		KTN_PROFILE_FUNCTION();

		NodeIndex parent1 = m_Nodes[p_Node1].Parent;
		NodeIndex parent2 = m_Nodes[p_Node2].Parent;

		if (parent1 == parent2)
		{
			m_Nodes[parent1].Child1 = p_Node2;
			m_Nodes[parent1].Child2 = p_Node1;
			return;
		}

		if (m_Nodes[parent1].Child1 == p_Node1)
			m_Nodes[parent1].Child1 = p_Node2;
		else
			m_Nodes[parent1].Child2 = p_Node2;

		m_Nodes[p_Node2].Parent = parent1;

		if (m_Nodes[parent2].Child1 == p_Node2)
			m_Nodes[parent2].Child1 = p_Node1;
		else
			m_Nodes[parent2].Child2 = p_Node1;

		m_Nodes[p_Node1].Parent = parent2;
	}


} // namespace KTN
