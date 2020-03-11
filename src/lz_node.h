#pragma once

#include "span.h"
#include <memory>

struct LzMatch
{
    std::size_t distance{0};
    std::size_t length{0};

    LzMatch() = default;

    explicit LzMatch(const std::size_t dist, std::size_t len) : distance(dist), length(len)
    {}

    [[nodiscard]] bool operator<(const LzMatch &other) const
    {
        return (length < other.length);
    }

    [[nodiscard]] bool operator>(const LzMatch &other) const
    {
        return (distance > other.distance);
    }
};

template<typename T = azgra::byte>
class LzNode
{
private:
    template<typename X>
    friend
    class LzTree;

    span<T> m_nodeDataSpan{};
    long m_distance{-1};
    LzNode<T> *m_parent{};
    std::unique_ptr<LzNode<T>> m_lesser{};
    std::unique_ptr<LzNode<T>> m_greater{};

    void traverse_best_match(const span<T> &targetData, LzMatch &bestMatch) const
    {
        if (m_lesser && (targetData < m_nodeDataSpan))
        {
            m_lesser->traverse_best_match(targetData, bestMatch);
        }
        else if (m_greater)
        {
            m_greater->traverse_best_match(targetData, bestMatch);
        }

        const std::size_t matchLength = m_nodeDataSpan.match_length(targetData);
        if (matchLength > bestMatch.length)
        {
            bestMatch.distance = m_distance;
            bestMatch.length = matchLength;
        }
    }

    LzNode<T> *find_node_by_data(const span<T> &targetNodeData)
    {
        if (m_nodeDataSpan == targetNodeData)
        {
            return this;
        }
        else if (m_lesser && (targetNodeData < m_nodeDataSpan))
        {
            return m_lesser->find_node_by_data(targetNodeData);
        }
        else if (m_greater)
        {
            return m_greater->find_node_by_data(targetNodeData);
        }
        return nullptr;
    }

    bool delete_node_from_parent(LzNode<T> *node)
    {
        if (m_lesser && (m_lesser.get() == node))
        {
            m_lesser.release();
            return true;
        }
        if (m_greater && (m_greater.get() == node))
        {
            m_greater.release();
            return true;
        }
        return false;
    }

    bool swap_parent_child(LzNode<T> *oldChild, std::unique_ptr<LzNode<T>> &newChild)
    {
        if (m_lesser && (m_lesser.get() == oldChild))
        {
            m_lesser = std::move(newChild);
            return true;
        }
        if (m_greater && (m_greater.get() == oldChild))
        {
            m_greater = std::move(newChild);
            return true;
        }
        return false;
    }

    LzNode<T> *find_left_most_on_right_side(LzNode<T> *node) const
    {
        if (node->m_greater)
        {
            return find_left_most_on_right_side(node->m_greater.get());
        }
        else if (node->m_lesser)
        {
            return node->m_lesser.get();
        }
        else
        {
            return node;
        }
    }

    [[nodiscard]] LzMatch find_best_match(const span<T> &data) const
    {
        LzMatch bestMatch;
        traverse_best_match(data, bestMatch);
        return bestMatch;
    }

    bool delete_node(const span<T> &dataToDelete)
    {
        auto nodeToDelete = find_node_by_data(dataToDelete);
        if (!nodeToDelete)
            return false;

        if (!nodeToDelete->m_lesser && !nodeToDelete->m_greater) // Node doesn't have children.
        {
            return nodeToDelete->m_parent->delete_node_from_parent(nodeToDelete);
        }
        else if (nodeToDelete->m_lesser && !nodeToDelete->m_greater) // Node has only lesser child
        {
            return swap_parent_child(nodeToDelete, nodeToDelete->m_lesser);
        }
        else if (nodeToDelete->m_greater && !nodeToDelete->m_lesser) // Node has only greater child
        {
            return swap_parent_child(nodeToDelete, nodeToDelete->m_greater);
        }
        else // Node has both children.
        {
            LzNode<T> *replacementNode = find_left_most_on_right_side(nodeToDelete);
            nodeToDelete->m_nodeDataSpan = replacementNode->m_nodeDataSpan;
            nodeToDelete->m_distance = replacementNode->m_distance;

            replacementNode->m_parent->delete_node_from_parent(replacementNode);
            return true;
        }
    }

    void add_child(std::unique_ptr<LzNode<T>> &newNode)
    {
        if (newNode->m_nodeDataSpan < m_nodeDataSpan)
        {
            if (!m_lesser)
            {
                newNode->m_parent = this;
                m_lesser = std::move(newNode);
            }
            else
            {
                m_lesser->add_child(newNode);
            }
        }
        else
        {
            if (!m_greater)
            {
                newNode->m_parent = this;
                m_greater = std::move(newNode);
            }
            else
            {
                m_greater->add_child(newNode);
            }
        }
    }

public:
    LzNode() = default;

    explicit LzNode(const span<T> data, const long distance_)
            : m_nodeDataSpan(data), m_distance(distance_)
    {
    }


};

typedef LzNode<azgra::byte> ByteLzNode;

