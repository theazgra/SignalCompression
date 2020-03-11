#pragma once

#include "lz_node.h"

template<typename T>
class LzTree
{
private:
    LzNode<T> m_root;

public:
    explicit LzTree(LzNode<T> &rootNode)
    {
        m_root = std::move(rootNode);
    }

    explicit LzTree(const span<T> &rootDataSpan, const std::size_t rootDistance)
    {
        m_root = LzNode<T>(rootDataSpan, rootDistance);
    }

    [[nodiscard]] LzMatch find_best_match(const span<T> &data) const
    {
        return m_root.find_best_match(data);
    }

    bool delete_node(const span<T> &dataToDelete)
    {
        return m_root.delete_node(dataToDelete);
    }

    void add_child(std::unique_ptr<LzNode<T>> &newNode)
    {
        m_root.add_child(newNode);
    }

};

typedef LzTree<azgra::byte> ByteLzTree;