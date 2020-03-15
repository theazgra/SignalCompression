#pragma once

#include "Span.h"
#include <memory>

/**
 * Match in the LZSS Binary Search Tree.
 */
struct LzMatch
{
    /**
     * Distance to the matched string.
     */
    std::size_t distance{0};
    /**
     * Length of the matched string.
     */
    std::size_t length{0};

    LzMatch() = default;

    /**
     * Create the match.
     * @param dist Distance to the matched string.
     * @param len Length of the match.
     */
    explicit LzMatch(const std::size_t dist, std::size_t len) : distance(dist), length(len)
    {}
};

/**
 * Flags for node deletion results.
 */
enum NodeDeletionResult
{
    NodeDeleted,
    NodeSurvived,
    NodeNotFound,
    UnknownError
};

/**
 * Node of the LZSS Binary Search Tree.
 * @tparam T Type of the node data.
 */
template<typename T = azgra::byte>
class LzNode
{
private:
    template<typename X>
    friend
    class LzTree;

    /**
     * Non-owning node data.
     */
    Span<T> m_data{};

    /**
     * Pointer to the node parent.
     */
    LzNode<T> *m_parent{nullptr};

    /**
     * Lesser (left) child.
     */
    std::unique_ptr<LzNode<T>> m_lesser{};

    /**
     * Greater (right) child.
     */
    std::unique_ptr<LzNode<T>> m_greater{};

    /**
     * Number of lives of the current node.
     */
    int m_lives{1};


    /**
     * Find node in the subtree of current node by its data.
     * @param targetNodeData Target node data.
     * @return Pointer to found node.
     */
    LzNode<T> *find_node_by_data(const Span<T> &targetNodeData)
    {
        int compare = targetNodeData.lexicographic_compare(m_data);
        if (compare == 0)
        {
            return this;
        }
        if (m_lesser && (compare < 0))
        {
            return m_lesser->find_node_by_data(targetNodeData);
        }
        if (m_greater && (compare > 0))
        {
            return m_greater->find_node_by_data(targetNodeData);
        }
        else
        {
            // NOTE(Moravec): This should not happen.
            assert(false);
            return nullptr;
        }
    }

    /**
     * Delete child node from this node.
     * @param node Node to delete.
     * @return Result of deletion.
     */
    NodeDeletionResult delete_child(LzNode<T> *node)
    {
        if (m_lesser && (m_lesser.get() == node))
        {
            assert(m_lesser->m_data == node->m_data);
            m_lesser.release();
            return NodeDeletionResult::NodeDeleted;
        }
        if (m_greater && (m_greater.get() == node))
        {
            assert(m_greater->m_data == node->m_data);
            m_greater.release();
            return NodeDeletionResult::NodeDeleted;
        }
        return NodeDeletionResult::NodeNotFound;
    }

    /**
     * Swap child of this node.
     * @param oldChild The old child node.
     * @param newChild The new child / replacement node.
     * @return True if node was swapped.
     */
    bool swap_child(LzNode<T> *oldChild, std::unique_ptr<LzNode<T>> &newChild)
    {
        if (m_lesser && (m_lesser.get() == oldChild))
        {
            assert(m_lesser->m_data == oldChild->m_data);
            assert(newChild->m_data.lexicographic_compare(m_data) < 0);

            m_lesser = std::move(newChild);
            m_lesser->m_parent = this;
            return true;
        }
        if (m_greater && (m_greater.get() == oldChild))
        {
            assert(m_greater->m_data == oldChild->m_data);
            assert(m_data.lexicographic_compare(newChild->m_data) < 0);

            m_greater = std::move(newChild);
            m_greater->m_parent = this;
            return true;
        }
        return false;
    }

    /**
     * Find inorder (the smallest greater node.) successor if this node.
     * @return Pointer to the inorder successor.
     */
    LzNode<T> *find_inorder_successor() const
    {
        assert(m_greater);
        auto *successor = m_greater.get();
        while (successor->m_lesser)
        {
            successor = successor->m_lesser.get();
        }
        return successor;
    }

    // TODO(Moravec): FIXME
    /**
     * Find best match for the target data. Will sue data.size for comparision.
     * @param targetData Target data.
     * @return Best match.
     */
    void find_best_match(const Span<T> &targetData, LzMatch &bestMatch) const
    {
        if (m_data[0] == targetData[0])
        {
            const std::size_t matchLength = m_data.match_length(targetData);
            if (matchLength > bestMatch.length)
            {
                bestMatch.distance = targetData.ptr - m_data.ptr;
                bestMatch.length = matchLength;
            }
        }

        const int compare = targetData.lexicographic_compare(m_data);
        if (m_lesser && (compare < 0)) // This node has lesser child and target data are smaller.
        {
            m_lesser->find_best_match(targetData, bestMatch);
        }
        else if (m_greater && (compare > 0)) // This node has greater child and target data are greater.
        {
            m_greater->find_best_match(targetData, bestMatch);
        }
    }

    /**
     * Delete node from this node or its subtree.
     * @param dataToDelete Target node data.
     * @param forceDeletion True if to force the deletion, ignore node lives.
     * @return Node deletion result.
     */
    NodeDeletionResult delete_node(const Span<T> &dataToDelete, const bool forceDeletion = false)
    {
        // Find node to delete.
        auto nodeToDelete = find_node_by_data(dataToDelete);
        if (!nodeToDelete)
        {
            always_assert(false && "Didn't find node to delete.");
            return NodeDeletionResult::NodeNotFound;
        }

        if (--nodeToDelete->m_lives > 0)
        {
            // NOTE(Moravec): Node still have one or more lives. Node won't be deleted
            if (!forceDeletion)
            {
                return NodeDeletionResult::NodeSurvived;
            }
        }

        if (!nodeToDelete->m_lesser && !nodeToDelete->m_greater) // Node doesn't have children.
        {
            auto parent = nodeToDelete->m_parent;
            const NodeDeletionResult result = parent->delete_child(nodeToDelete);
            assert(result == NodeDeletionResult::NodeDeleted);

            if (m_lesser)
                assert(m_lesser->m_data.lexicographic_compare(m_data) < 0);
            if (m_greater)
                assert(m_data.lexicographic_compare(m_greater->m_data) < 0);

            return result;
        }
        else if (nodeToDelete->m_lesser && !nodeToDelete->m_greater) // Node has only lesser child
        {
            auto parent = nodeToDelete->m_parent;
            assert(parent != nullptr);
            auto newChild = std::move(nodeToDelete->m_lesser);
            const bool result = parent->swap_child(nodeToDelete, newChild);
            assert(result);

            if (m_lesser)
                assert(m_lesser->m_data.lexicographic_compare(m_data) < 0);
            if (m_greater)
                assert(m_data.lexicographic_compare(m_greater->m_data) < 0);

            return result ? NodeDeletionResult::NodeDeleted : NodeDeletionResult::NodeNotFound;
        }
        else if (nodeToDelete->m_greater && !nodeToDelete->m_lesser) // Node has only greater child
        {
            auto parent = nodeToDelete->m_parent;
            assert(parent != nullptr);
            auto newChild = std::move(nodeToDelete->m_greater);
            const bool result = parent->swap_child(nodeToDelete, newChild);

            if (m_lesser)
                assert(m_lesser->m_data.lexicographic_compare(m_data) < 0);
            if (m_greater)
                assert(m_data.lexicographic_compare(m_greater->m_data) < 0);

            assert(result);
            return result ? NodeDeletionResult::NodeDeleted : NodeDeletionResult::NodeNotFound;
        }
        else // Node has both children.
        {
            // Find inorder successor.
            LzNode<T> *successor = nodeToDelete->find_inorder_successor();
            assert(successor != nullptr);

            const auto successorSpan = successor->m_data;
            const int successorLives = successor->m_lives;

            const NodeDeletionResult successorDeletion = delete_node(successorSpan, true);
            assert(successorDeletion == NodeDeletionResult::NodeDeleted);

            nodeToDelete->m_data = successorSpan;
            nodeToDelete->m_lives = successorLives;

            if (m_lesser)
                assert(m_lesser->m_data.lexicographic_compare(m_data) < 0);
            if (m_greater)
                assert(m_data.lexicographic_compare(m_greater->m_data) < 0);

            return successorDeletion;
        }
    }

    /**
     * Add child to the current node or its subtree.
     * @param nodeData New node data.
     */
    void add_child(Span<T> &&nodeData)
    {
        // Compare new node data to this node data.
        const int compare = nodeData.lexicographic_compare(m_data);
        if (compare == 0)
        {
            // NOTE(Moravec): Data is duplicate, update this node lives and data.
            m_data = nodeData;
            m_lives += 1;
        }
        else if (compare < 0)
        {
            if (!m_lesser)
            {
                m_lesser = std::make_unique<LzNode<T>>(std::move(nodeData));
                m_lesser->m_parent = this;
                assert(m_lesser->m_data.lexicographic_compare(m_data) < 0);
            }
            else
            {
                m_lesser->add_child(std::move(nodeData));
            }
        }
        else
        {
            assert(compare > 0);
            if (!m_greater)
            {
                m_greater = std::make_unique<LzNode<T>>(std::move(nodeData));
                m_greater->m_parent = this;
                assert(m_data.lexicographic_compare(m_greater->m_data) < 0);
            }
            else
            {
                m_greater->add_child(std::move(nodeData));
            }
        }
    }

public:
    /**
     * Default node constructor.
     */
    LzNode() = default;

    /**
     * Create node with its data.
     * @param data Node data.
     */
    explicit LzNode(Span<T> &&data)
    {
        m_data = std::move(data);
    }

    /**
     * Check if this node or its subtree has this data.
     * @param nodeData Target node data.
     * @return True if node with data exists.
     */
    [[nodiscard]] bool has_node_with_data(const Span<T> &nodeData)
    {
        const auto node = find_node_by_data(nodeData);
        return (node != nullptr);
    }

};

/**
 * Typedef to memory node.
 */
typedef LzNode<azgra::byte> ByteLzNode;

