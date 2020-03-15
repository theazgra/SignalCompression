#pragma once

#include "lz_node.h"

/**
 * LZSS Binary Search tree.
 * @tparam T Type of the node data.
 */
template<typename T>
class LzTree
{
private:
    /**
     * Tree root node.
     */
    std::unique_ptr<LzNode<T>> m_root;

    /**
     * Node count in the tree.
     */
    std::size_t m_nodeCount{0};

public:
    /**
     * Default constructor. Tree without root.
     */
    LzTree() = default;

    /**
     * Create the tree with the root.
     * @param rootNode The root node.
     */
    explicit LzTree(std::unique_ptr<LzNode<T>> &&rootNode)
    {
        m_root = std::move(rootNode);
        m_nodeCount = 1;
    }

    /**
     * Find best match for the targetData.
     * @param targetData Data to find match for.
     * @return Best match for the data.
     */
    [[nodiscard]] LzMatch find_best_match(const Span<T> &targetData) const
    {
        if (!m_root)
        { return LzMatch(); }

        LzMatch match{};
        m_root->find_best_match(targetData, match);
        return match;
    }

    /**
     * Delete node from the tree, based on its data.
     * @param dataToDelete Node to be deleted data.
     * @return Deletion result
     */
    NodeDeletionResult delete_node(const Span<T> &dataToDelete)
    {
        if (!m_root)
        { return NodeDeletionResult::NodeNotFound; }

        NodeDeletionResult deletionResult = NodeDeletionResult::UnknownError;
        // Deleting root node.
        if (m_root->m_data == dataToDelete)
        {
            if (--m_root->m_lives >= 1)
            {
                // NOTE(Moravec): Node still have one or more lives. Node won't be deleted
                return NodeDeletionResult::NodeSurvived;
            }
            assert(m_root->m_lesser || m_root->m_greater);

            if (m_root->m_lesser && !m_root->m_greater)
            {
                auto lesserNode = std::move(m_root->m_lesser);
                m_root.release();
                m_root = std::move(lesserNode);
                m_root->m_parent = nullptr;

                deletionResult = NodeDeletionResult::NodeDeleted;
                if (m_root->m_lesser)
                {
                    assert(m_root->m_lesser->m_data.lexicographic_compare(m_root->m_data) <= 0);
                }
                if (m_root->m_greater)
                {
                    assert(m_root->m_greater->m_data.lexicographic_compare(m_root->m_data) > 0);
                }
            }
            else if (m_root->m_greater && !m_root->m_lesser)
            {
                auto greaterNode = std::move(m_root->m_greater);
                m_root.release();
                m_root = std::move(greaterNode);
                m_root->m_parent = nullptr;

                deletionResult = NodeDeletionResult::NodeDeleted;

                if (m_root->m_lesser)
                {
                    assert(m_root->m_lesser->m_data.lexicographic_compare(m_root->m_data) <= 0);
                }
                if (m_root->m_greater)
                {
                    assert(m_root->m_greater->m_data.lexicographic_compare(m_root->m_data) > 0);
                }
            }
            else
            {
                LzNode<T> *successor = m_root->find_inorder_successor();
                const auto successorSpan = successor->m_data;
                const auto successorLives = successor->m_lives;

                assert(m_root->m_data.lexicographic_compare(successorSpan) < 0);

                const NodeDeletionResult deletedSuccessorNode = m_root->delete_node(successorSpan, true);
                assert(deletedSuccessorNode == NodeDeletionResult::NodeDeleted);

                deletionResult = deletedSuccessorNode;

                m_root->m_data = successorSpan;
                m_root->m_lives = successorLives;
            }
        }
        else
        {
            deletionResult = m_root->delete_node(dataToDelete);
        }

        if (deletionResult == NodeDeletionResult::NodeDeleted)
        {
            --m_nodeCount;
        }
        return deletionResult;
    }

    /**
     * Add new node to the tree.
     * @param nodeData New node data.
     */
    void add_node(Span<T> &&nodeData)
    {
        if (!m_root)
        {
            m_root = std::make_unique<LzNode<T>>(std::move(nodeData));
        }
        else
        {

            m_root->add_child(std::move(nodeData));
        }
        ++m_nodeCount;
    }

    /**
     * Check if tree contains node with targetData.
     * @param targetData Target node data.
     * @return True if node exists.
     */
    [[nodiscard]] bool contains_node(const Span<T> &targetData)
    {
        return m_root->has_node_with_data(targetData);
    }

};

/**
 * Typedef for memory tree.
 */
typedef LzTree<azgra::byte> ByteLzTree;