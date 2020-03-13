#pragma once

#include "lz_node.h"

template<typename T>
class LzTree
{
private:
    std::unique_ptr<LzNode<T>> m_root;

public:
    LzTree() = default;

    explicit LzTree(std::unique_ptr<LzNode<T>> &&rootNode)
    {
        m_root = std::move(rootNode);
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
        return m_root->delete_node(dataToDelete);
    }

    void add_child(std::unique_ptr<LzNode<T>> &newNode)
    {
        if (!m_root)
        {
            m_root = std::move(newNode);
        }
        m_root->add_child(newNode);
    }

};

typedef LzTree<azgra::byte> ByteLzTree;