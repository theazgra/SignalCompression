#pragma once

#include "lz_node.h"

template<typename T>
class LzTree
{
private:
    std::unique_ptr<LzNode<T>> m_root;
    std::size_t m_nodeCount{0};

public:
    LzTree() = default;

    explicit LzTree(std::unique_ptr<LzNode<T>> &&rootNode)
    {
        m_root = std::move(rootNode);
        m_nodeCount = 1;
    }

    explicit LzTree(const span<T> &rootDataSpan, const std::size_t rootDistance) : LzTree(LzNode<T>(rootDataSpan, rootDistance))
    {
    }

    [[nodiscard]] LzMatch find_best_match(const span<T> &data) const
    {
        if (!m_root)
        { return LzMatch(); }
        return m_root->find_best_match(data);
    }

    bool delete_node(const span<T> &dataToDelete)
    {
        if (!m_root)
        { return false; }

        bool deleted = false;
        // Deleting root node.
        if (m_root->m_nodeDataSpan == dataToDelete)
        {
            assert(m_root->m_lesser || m_root->m_greater);

            if (m_root->m_lesser && !m_root->m_greater)
            {
                auto lesserNode = std::move(m_root->m_lesser);
                m_root.release();
                m_root = std::move(lesserNode);
                m_root->m_isRoot = true;
                m_root->m_parent = nullptr;

                deleted = true;

                if (m_root->m_lesser)
                {
                    assert(m_root->m_lesser->m_nodeDataSpan.compare(m_root->m_nodeDataSpan) <= 0);
                }
                if (m_root->m_greater)
                {
                    assert(m_root->m_greater->m_nodeDataSpan.compare(m_root->m_nodeDataSpan) > 0);
                }
            }
            else if (m_root->m_greater && !m_root->m_lesser)
            {
                auto greaterNode = std::move(m_root->m_greater);
                m_root.release();
                m_root = std::move(greaterNode);
                m_root->m_isRoot = true;
                m_root->m_parent = nullptr;

                deleted = true;

                if (m_root->m_lesser)
                {
                    assert(m_root->m_lesser->m_nodeDataSpan.compare(m_root->m_nodeDataSpan) <= 0);
                }
                if (m_root->m_greater)
                {
                    assert(m_root->m_greater->m_nodeDataSpan.compare(m_root->m_nodeDataSpan) > 0);
                }
            }
            else
            {
                LzNode<T> *successor = m_root->find_inorder_successor();
                const auto successorSpan = successor->m_nodeDataSpan;
                assert(m_root->m_nodeDataSpan.compare(successorSpan) < 0);

                const bool deletedSuccessorNode = delete_node(successorSpan);
                assert(deletedSuccessorNode);

                m_root->m_nodeDataSpan = successorSpan;
                deleted = true;

            }
        }
        else
        {
            deleted = m_root->delete_node(dataToDelete);
        }


        if (deleted)
        {
            --m_nodeCount;
        }
        return deleted;
    }

    void add_node(std::unique_ptr<LzNode<T>> &&newNode)
    {
        if (!m_root)
        {
            m_root = std::move(newNode);
            m_root->m_isRoot = true;
        }
        else
        {
            m_root->add_child(newNode);
        }
        ++m_nodeCount;
    }

    [[nodiscard]] bool has_node_with_data(const span<T> &nodeData)
    {
        return m_root->has_node_with_data(nodeData);
    }

};

typedef LzTree<azgra::byte> ByteLzTree;