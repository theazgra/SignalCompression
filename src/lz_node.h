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
    LzNode<T> *m_parent{};
    std::unique_ptr<LzNode<T>> m_lesser{};
    std::unique_ptr<LzNode<T>> m_greater{};
    bool m_isRoot{false};

    void traverse_best_match(const span<T> &targetData, LzMatch &bestMatch) const
    {
        const int compare = targetData.compare(m_nodeDataSpan);
        if (compare == 0)
        {
            bestMatch.distance = targetData.ptr - m_nodeDataSpan.ptr;
            bestMatch.length = targetData.size;
            return;
        }
        else if (m_lesser && (compare < 0))
        {
            m_lesser->traverse_best_match(targetData, bestMatch);
        }
        else if (m_greater && (compare > 0))
        {
            m_greater->traverse_best_match(targetData, bestMatch);
        }
        else
        {
            const std::size_t matchLength = m_nodeDataSpan.match_length(targetData);
            if (matchLength > bestMatch.length)
            {
                bestMatch.distance = targetData.ptr - m_nodeDataSpan.ptr;
                bestMatch.length = matchLength;
            }
        }
    }


    LzNode<T> *find_node_by_data(const span<T> &targetNodeData)
    {
        int compare = targetNodeData.compare(m_nodeDataSpan);
        if (compare == 0)
        {
            return this;
        }
        if (m_lesser && (compare <= 0))
        {
            return m_lesser->find_node_by_data(targetNodeData);
        }
        if (m_greater && (compare > 0))
        {
            return m_greater->find_node_by_data(targetNodeData);
        }
        else return nullptr;
    }

    bool delete_child(LzNode<T> *node)
    {
        if (m_lesser && (m_lesser.get() == node))
        {
            assert(m_lesser->m_nodeDataSpan == node->m_nodeDataSpan);
            m_lesser.release();
            return true;
        }
        if (m_greater && (m_greater.get() == node))
        {
            assert(m_greater->m_nodeDataSpan == node->m_nodeDataSpan);
            m_greater.release();
            return true;
        }
        return false;
    }

    bool swap_my_child(LzNode<T> *oldChild, std::unique_ptr<LzNode<T>> &newChild)
    {
        if (m_lesser && (m_lesser.get() == oldChild))
        {
            assert(m_lesser->m_nodeDataSpan == oldChild->m_nodeDataSpan);
            assert(newChild->m_nodeDataSpan.compare(m_nodeDataSpan) <= 0);

            m_lesser = std::move(newChild);
            m_lesser->m_parent = this;
            return true;
        }
        if (m_greater && (m_greater.get() == oldChild))
        {
            assert(m_greater->m_nodeDataSpan == oldChild->m_nodeDataSpan);
            assert(m_nodeDataSpan.compare(newChild->m_nodeDataSpan) <= 0);

            m_greater = std::move(newChild);
            m_greater->m_parent = this;
            return true;
        }
        return false;
    }

    LzNode<T> *find_inorder_successor() const
    {
        always_assert(m_greater);

        auto *successor = m_greater.get();
        while (successor->m_lesser)
        {
            successor = successor->m_lesser.get();
        }
        return successor;
    }

//    LzNode<T> *find_left_most_on_right_side(LzNode<T> *node) const
//    {
//        if (node->m_greater)
//        {
//            return find_left_most_on_right_side(node->m_greater.get());
//        }
//        else if (node->m_lesser)
//        {
//            return node->m_lesser.get();
//        }
//        else
//        {
//            return node;
//        }
//    }

    [[nodiscard]] LzMatch find_best_match(const span<T> &data) const
    {
        LzMatch bestMatch;
        traverse_best_match(data, bestMatch);
        return bestMatch;
    }

    bool delete_node(const span<T> &dataToDelete, LzNode<T> *ancestor = nullptr)
    {
        auto nodeToDelete = (ancestor != nullptr) ? ancestor->find_node_by_data(dataToDelete) : find_node_by_data(dataToDelete);
        if (!nodeToDelete)
        {
            assert(false && "Didn't find node to delete.");
            return false;
        }

        if (!nodeToDelete->m_lesser && !nodeToDelete->m_greater) // Node doesn't have children.
        {
            auto parent = nodeToDelete->m_parent;
            const bool result = parent->delete_child(nodeToDelete);
            assert(result);
            return result;
        }
        else if (nodeToDelete->m_lesser && !nodeToDelete->m_greater) // Node has only lesser child
        {
            auto parent = nodeToDelete->m_parent;
            assert(parent != nullptr);
            auto newChild = std::move(nodeToDelete->m_lesser);
            const bool result = parent->swap_my_child(nodeToDelete, newChild);

            assert(result);
            return result;
        }
        else if (nodeToDelete->m_greater && !nodeToDelete->m_lesser) // Node has only greater child
        {
            auto parent = nodeToDelete->m_parent;
            assert(parent != nullptr);
            auto newChild = std::move(nodeToDelete->m_greater);
            const bool result = parent->swap_my_child(nodeToDelete, newChild);



            assert(result);
            return result;
        }
        else // Node has both children.
        {
//            return true;
            LzNode<T> *successor = nodeToDelete->find_inorder_successor();
            const auto successorSpan = successor->m_nodeDataSpan;

            const bool deletedSuccessorNode = delete_node(successorSpan, nodeToDelete->m_greater.get());
            assert(deletedSuccessorNode);

            nodeToDelete->m_nodeDataSpan = successorSpan;
            return true;
        }
    }

    void add_child(std::unique_ptr<LzNode<T>> &newNode)
    {
        if (newNode->m_nodeDataSpan.compare(m_nodeDataSpan) <= 0)
        {
            if (!m_lesser)
            {
                newNode->m_parent = this;
                m_lesser = std::move(newNode);
                assert(m_lesser->m_nodeDataSpan.compare(m_nodeDataSpan) <= 0);
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
                assert(m_nodeDataSpan.compare(m_greater->m_nodeDataSpan) <= 0);
            }
            else
            {
                m_greater->add_child(newNode);
            }
        }
    }

public:
    LzNode() = default;

    explicit LzNode(span<T> &&data)
    {
        m_nodeDataSpan = std::move(data);
    }


    [[nodiscard]] bool has_node_with_data(const span<T> &nodeData)
    {
        const auto node = find_node_by_data(nodeData);
        return (node != nullptr);
    }


};

typedef LzNode<azgra::byte> ByteLzNode;

