#include "ktnpch.h"
#include "BVHTree.h"
#include "Koten/Core/ThreadManager.h"

// std
#include <thread>


namespace KTN::Math
{
    BVHTree::BVHTree(const BVHSettings& p_Settings)
        : m_Settings(p_Settings)
    {
        m_Root = CreateUnique<Node>();
    }

    void BVHTree::SetSettings(const BVHSettings& p_Settings)
    {
        std::lock_guard lock(m_Mutex);
        m_Settings = p_Settings;
    }

    bool BVHTree::Insert(BVHObjectID p_ID, const BoundingBox& p_Bounds)
    {
        KTN_PROFILE_FUNCTION();
        std::lock_guard lock(m_Mutex);

        if (p_ID == INVALID_BVH_ID || !p_Bounds.IsValid())
            return false;

        auto it = m_ObjectLeaves.find(p_ID);
        if (it != m_ObjectLeaves.end())
        {
            Node* leaf = it->second;
            auto objectIt = std::find_if(leaf->Objects.begin(), leaf->Objects.end(),
            [p_ID](const Object& object)
            {
                return object.ID == p_ID;
            });

            if (objectIt != leaf->Objects.end())
            {
                if (objectIt->Bounds == p_Bounds)
                    return true;

                if (objectIt->FatBounds.Contains(p_Bounds))
                {
                    objectIt->Bounds = p_Bounds;
                    return true;
                }
                else
                {
                    objectIt->Bounds = p_Bounds;
                    objectIt->FatBounds = CreateFatBounds(p_Bounds);
                    return true;
                }
            }

            if (!RemoveInternal(p_ID))
                return false;
        }

        Object object;
        object.ID        = p_ID;
        object.Bounds    = p_Bounds;
        object.FatBounds = CreateFatBounds(p_Bounds);

        return InsertInto(m_Root.get(), object, 0);
    }

    bool BVHTree::Update(BVHObjectID p_ID, const BoundingBox& p_Bounds)
    {
        KTN_PROFILE_FUNCTION();
        return Insert(p_ID, p_Bounds);
    }

    bool BVHTree::Remove(BVHObjectID p_ID)
    {
        KTN_PROFILE_FUNCTION();
        std::lock_guard lock(m_Mutex);

        return RemoveInternal(p_ID);
    }

    void BVHTree::Query(const BVHIntersectsFn& p_Intersects, const BVHCallbackFn& p_Callback)
    {
        KTN_PROFILE_FUNCTION();

        if (!m_Root || m_ObjectLeaves.empty())
            return;

        constexpr size_t ASYNC_THRESHOLD = 512;
        if (m_ObjectLeaves.size() >= ASYNC_THRESHOLD)
            QueryNodeAsync(m_Root.get(), p_Intersects, p_Callback);
        else
            QueryNode(m_Root.get(), p_Intersects, p_Callback);
    }

    void BVHTree::Clear()
    {
        KTN_PROFILE_FUNCTION_LOW();
        std::lock_guard lock(m_Mutex);

        m_Root = CreateUnique<Node>();
        m_ObjectLeaves.clear();
    }

    bool BVHTree::Contains(BVHObjectID p_ID) const
    {
        std::lock_guard lock(m_Mutex);
        return m_ObjectLeaves.contains(p_ID);
    }

    size_t BVHTree::GetObjectCount() const
    {
        std::lock_guard lock(m_Mutex);
        return m_ObjectLeaves.size();
    }

    BVHSettings BVHTree::GetSettings() const
    {
        std::lock_guard lock(m_Mutex);
        return m_Settings;
    }

    BoundingBox BVHTree::CreateFatBounds(const BoundingBox& p_Bounds) const
    {
        KTN_PROFILE_FUNCTION_LOW();

        if (!p_Bounds.IsValid())
            return {};

        const glm::vec3 size   = p_Bounds.GetSize();
        const glm::vec3 margin = glm::vec3(m_Settings.FatBoundsMargin) + size * m_Settings.FatBoundsRelativeMargin;
        return BoundingBox(p_Bounds.GetMin() - margin, p_Bounds.GetMax() + margin);
    }

    bool BVHTree::RemoveInternal(BVHObjectID p_ID)
    {
        KTN_PROFILE_FUNCTION_LOW();

        if (p_ID == INVALID_BVH_ID)
            return false;

        auto it = m_ObjectLeaves.find(p_ID);
        if (it == m_ObjectLeaves.end())
            return true;

        Node* leaf = it->second;
        auto objectIt = std::find_if(leaf->Objects.begin(), leaf->Objects.end(), [p_ID](const Object& object)
        {
            return object.ID == p_ID;
        });

        if (objectIt == leaf->Objects.end())
        {
            m_ObjectLeaves.erase(it);
            return true;
        }

        leaf->Objects.erase(objectIt);
        m_ObjectLeaves.erase(it);

        if (!RefitUpwards(leaf))
            return false;

        if (!TryMergeUpwards(leaf))
            return false;

        return true;
    }

    bool BVHTree::InsertInto(Node* p_Node, const Object& p_Object, uint32_t p_Depth)
    {
        KTN_PROFILE_FUNCTION_LOW();

        if (!p_Node || !p_Object.IsValid() || !p_Object.FatBounds.IsValid() ||
           (p_Node->Objects.size() > m_Settings.MaxObjectsPerLeaf && p_Depth >= m_Settings.MaxDepth))
        {
            return false;
        }

        if (!p_Node->IsLeaf())
        {
            Node* child = ChooseBestChild(p_Node, p_Object.FatBounds);
            if (!InsertInto(child, p_Object, p_Depth + 1))
                return false;

            return RefitUpwards(child);
        }

        p_Node->Objects.push_back(p_Object);
        m_ObjectLeaves[p_Object.ID] = p_Node;

        p_Node->Bounds.Merge(p_Object.FatBounds);
        if (p_Node->Objects.size() > m_Settings.MaxObjectsPerLeaf && p_Depth < m_Settings.MaxDepth)
        {
            return Split(p_Node, p_Depth);
        }

        return true;
    }

    BVHTree::Node* BVHTree::ChooseBestChild(Node* p_Node, const BoundingBox& p_Bounds) const
    {
        KTN_PROFILE_FUNCTION_LOW();

        const auto leftUnion  = BoundingBox::Union(p_Node->Left->Bounds, p_Bounds);
        const auto rightUnion = BoundingBox::Union(p_Node->Right->Bounds, p_Bounds);

        const float leftCost  = leftUnion.GetSurfaceArea() - p_Node->Left->Bounds.GetSurfaceArea();
        const float rightCost = rightUnion.GetSurfaceArea() - p_Node->Right->Bounds.GetSurfaceArea();

        return leftCost <= rightCost ? p_Node->Left.get() : p_Node->Right.get();
    }

    bool BVHTree::Split(Node* p_Node, uint32_t p_Depth)
    {
        KTN_PROFILE_FUNCTION_LOW();

        if (!p_Node || !p_Node->IsLeaf() || p_Node->Objects.size() <= 1)
            return false;

        if (p_Depth >= m_Settings.MaxDepth)
            return false;

        glm::vec3 centerMin( FLT_MAX);
        glm::vec3 centerMax(-FLT_MAX);

        for (const Object& object : p_Node->Objects)
        {
            const glm::vec3 center = object.FatBounds.GetCenter();

            centerMin = glm::min(centerMin, center);
            centerMax = glm::max(centerMax, center);
        }

        const glm::vec3 extend = centerMax - centerMin;

        uint32_t axis = 0;
        if (extend.y > extend.x && extend.y >= extend.z)
            axis      = 1;
        else if (extend.z > extend.x && extend.z > extend.y)
            axis      = 2;

        std::sort(p_Node->Objects.begin(), p_Node->Objects.end(), [axis](const Object& p_A, const Object& p_B)
        {
            return p_A.FatBounds.GetCenter()[axis] < p_B.FatBounds.GetCenter()[axis];
        });

        const size_t middle   = p_Node->Objects.size() / 2;

        p_Node->Left          = std::make_unique<Node>();
        p_Node->Right         = std::make_unique<Node>();

        p_Node->Left->Parent  = p_Node;
        p_Node->Right->Parent = p_Node;

        for (size_t i = 0; i < p_Node->Objects.size(); ++i)
        {
            Node* child = i < middle ? p_Node->Left.get() : p_Node->Right.get();

            child->Objects.push_back(std::move(p_Node->Objects[i]));
            child->Bounds.Merge(child->Objects.back().FatBounds);

            m_ObjectLeaves[child->Objects.back().ID] = child;
        }

        p_Node->Objects.clear();

        Refit(p_Node);

        if (p_Depth + 1 < m_Settings.MaxDepth)
        {
            if (p_Node->Left->Objects.size() > m_Settings.MaxObjectsPerLeaf)
            {
                if (!Split(p_Node->Left.get(), p_Depth + 1))
                    return false;
            }

            if (p_Node->Right->Objects.size() > m_Settings.MaxObjectsPerLeaf)
            {
                if (!Split(p_Node->Right.get(), p_Depth + 1))
                    return false;
            }
        }

        return Refit(p_Node);
    }

    bool BVHTree::Refit(Node* p_Node)
    {
        KTN_PROFILE_FUNCTION_LOW();

        if (!p_Node)
            return false;

        p_Node->Bounds = {};
        if (p_Node->IsLeaf())
        {
            for (const Object& object : p_Node->Objects)
                p_Node->Bounds.Merge(object.FatBounds);
        }
        else
        {
            if (p_Node->Left)
                p_Node->Bounds.Merge(p_Node->Left->Bounds);

            if (p_Node->Right)
                p_Node->Bounds.Merge(p_Node->Right->Bounds);
        }
        return true;
    }

    bool BVHTree::RefitUpwards(Node* p_Node)
    {
        KTN_PROFILE_FUNCTION_LOW();

        while (p_Node)
        {
            if (!Refit(p_Node))
                return false;

            p_Node = p_Node->Parent;
        }

        return true;
    }

    bool BVHTree::TryMergeUpwards(Node* p_Node)
    {
        KTN_PROFILE_FUNCTION_LOW();

        if (!p_Node)
            return false;

        while (p_Node)
        {
            Node* parent = p_Node->Parent;
            if (!parent)
                break;

            Node* left  = parent->Left.get();
            Node* right = parent->Right.get();

            if (left && right && left->IsLeaf() && right->IsLeaf() && left->Objects.size() + right->Objects.size() <= m_Settings.MaxObjectsPerLeaf)
            {
                parent->Objects.reserve(left->Objects.size() + right->Objects.size());
                for (Object& object : left->Objects)
                {
                    if (object.ID == INVALID_BVH_ID) continue;

                    m_ObjectLeaves[object.ID] = parent;
                    parent->Objects.push_back(std::move(object));
                }

                for (Object& object : right->Objects)
                {
                    if (object.ID == INVALID_BVH_ID) continue;

                    m_ObjectLeaves[object.ID] = parent;
                    parent->Objects.push_back(std::move(object));
                }

                parent->Left.reset();
                parent->Right.reset();
            }

            Refit(parent);
            p_Node = parent;
        }
        return true;
    }

    void BVHTree::QueryNode(const Node* p_Node, const BVHIntersectsFn& p_Intersects, const BVHCallbackFn& p_Callback, bool p_ParentInside)
    {
        KTN_PROFILE_FUNCTION_LOW();

        if (!p_Node || !p_Node->Bounds.IsValid())
            return;

        const Intersection result = p_ParentInside ? Intersection::Inside : p_Intersects(p_Node->Bounds);
        if (result == Intersection::Outside)
            return;

        if (p_Node->IsLeaf())
        {
            for (const Object& object : p_Node->Objects)
            {
                Intersection value = result;
                if (value != Intersection::Inside)
                    value = p_Intersects(object.Bounds);
                if (value != Intersection::Outside)
                {
                    std::lock_guard lock(m_CallbackMutex);
                    if (p_Callback(object.ID, object.Bounds))
                        return;
                }
            }
            return;
        }

        const bool inside = result == Intersection::Inside;
        QueryNode(p_Node->Left.get(), p_Intersects, p_Callback, inside);
        QueryNode(p_Node->Right.get(), p_Intersects, p_Callback, inside);
    }

    void BVHTree::QueryNodeAsync(const Node* p_Node, const BVHIntersectsFn& p_Intersects, const BVHCallbackFn& p_Callback, bool p_ParentInside)
    {
        KTN_PROFILE_FUNCTION();

        if (!p_Node || !p_Node->Bounds.IsValid())
            return;

        uint32_t workerCount = std::thread::hardware_concurrency();
        if (workerCount == 0)
            workerCount = 4;

        const size_t maxTasks = std::clamp(static_cast<size_t>(workerCount * 2), (size_t)2, (size_t)16);

        std::vector<std::pair<const Node*, bool>> frontier;
        frontier.emplace_back(p_Node, p_ParentInside);
        while (frontier.size() < maxTasks)
        {
            auto it = std::find_if(frontier.begin(), frontier.end(),
            [](const auto& entry)
            {
                return !entry.first->IsLeaf();
            });

            if (it == frontier.end())
                break;

            const Node* node   = it->first;
            const bool inside  = it->second;
            const size_t index = std::distance(frontier.begin(), it);

            frontier.erase(it);
            frontier.insert(frontier.begin() + index, {{ node->Right.get(), inside }, { node->Left.get(),  inside }});
        }

        std::vector<std::vector<Object>> results(frontier.size());

        auto group = JobGroup::Create();
        for (size_t i = 0; i < frontier.size(); ++i)
        {
            const auto [node, parentInside] = frontier[i];

            ThreadManager::Get().ScheduleJob(
            group, [&, i, node, parentInside]()
            {
                auto traverse = [&](auto&& self, const Node* current, bool inheritedInside) -> void
                {
                    if (!current || !current->Bounds.IsValid())
                        return;

                    const Intersection result = inheritedInside ? Intersection::Inside : p_Intersects(current->Bounds);
                    if (result == Intersection::Outside)
                        return;

                    if (current->IsLeaf())
                    {
                        for (const Object& object : current->Objects)
                        {
                            Intersection value = result;
                            if (value != Intersection::Inside)
                                value = p_Intersects(object.Bounds);

                            if (value != Intersection::Outside)
                                results[i].push_back(object);
                        }
                        return;
                    }

                    const bool inside = result == Intersection::Inside;

                    self(self, current->Left.get(), inside);
                    self(self, current->Right.get(), inside);
                };
                traverse(traverse, node, parentInside);
            });
        }

        group->Wait();
        for (const auto& list : results)
        {
            for (const Object& object : list)
            {
                std::lock_guard lock(m_CallbackMutex);

                p_Callback(object.ID, object.Bounds);
            }
        }
    }

} // namespace KTN::Math
