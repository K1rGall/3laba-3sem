#ifndef BTREE_H
#define BTREE_H

#include "IDictionary.h"
#include "ShrdPtr.h"
#include "DynamicArraySmart.h"
#include "UnqPtr.h"
#include <iostream>
#include <stdexcept>

template<typename TKey, typename TElement>
class BTree : public IDictionary<TKey, TElement> {
public:

    BTree(int order = 3);

    virtual ~BTree();

    virtual size_t GetCount() const override;

    virtual TElement Get(const TKey &key) const override;

    virtual bool ContainsKey(const TKey &key) const override;

    virtual void Add(const TKey &key, const TElement &element) override;

    virtual void Remove(const TKey &key) override;

    virtual UnqPtr<IDictionaryIterator<TKey, TElement>> GetIterator() const override;

    virtual TElement& operator[](const TKey &key) override;

private:
    struct Node {
        bool isLeaf;
        int numKeys;
        UnqPtr<TKey[]> keys;
        UnqPtr<TElement[]> values;
        UnqPtr<ShrdPtr<Node>[]> children;

        Node(bool leaf, int order);
    };

    ShrdPtr<Node> root;
    int order;
    size_t count;

    void SplitChild(ShrdPtr<Node> parent, int index);

    void InsertNonFull(ShrdPtr<Node> &node, const TKey &key, const TElement &value);

    TElement& FindOrInsert(ShrdPtr<Node> &node, const TKey &key);

    TElement Search(ShrdPtr<Node> x, const TKey &key) const;

    bool Contains(ShrdPtr<Node> x, const TKey &key) const;

    void RemoveFromNode(ShrdPtr<Node> x, const TKey &key);

    void RemoveFromLeaf(ShrdPtr<Node> x, int idx);

    void RemoveFromNonLeaf(ShrdPtr<Node> x, int idx);

    TKey GetPredecessor(ShrdPtr<Node> x, int idx);

    TKey GetSuccessor(ShrdPtr<Node> x, int idx);

    void Fill(ShrdPtr<Node> x, int idx);

    void BorrowFromPrev(ShrdPtr<Node> x, int idx);

    void BorrowFromNext(ShrdPtr<Node> x, int idx);

    void Merge(ShrdPtr<Node> x, int idx);

    class BTreeIterator : public IDictionaryIterator<TKey, TElement> {
    public:
        TElement& operator[](const TKey &key);

        BTreeIterator(const BTree *tree);

        virtual ~BTreeIterator() {}

        virtual bool MoveNext() override;

        virtual void Reset() override;

        virtual TKey GetCurrentKey() const override;

        virtual TElement GetCurrentValue() const override;

    private:
        const BTree *tree;
        struct StackNode {
            ShrdPtr<Node> node;
            int index;
        };
        DynamicArraySmart<StackNode> stack;
        TKey currentKey;
        TElement currentValue;
        bool hasCurrent;

        void PushLeftmost(ShrdPtr<Node> node);
    };

    friend class BTreeTest;

public:
    TElement& operator()(int row, int column);

    void PrintStructure(ShrdPtr<Node> node = ShrdPtr<Node>(), int depth = 0) const {
        auto currentNode = node ? node : root;
        if (!currentNode) return;

        for (int i = 0; i < depth; ++i) std::cout << "  ";
        std::cout << "[";

        for (int i = 0; i < currentNode->numKeys; ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << currentNode->keys[i];
        }
        std::cout << "]\n";

        if (!currentNode->isLeaf) {
            for (int i = 0; i <= currentNode->numKeys; ++i) {
                PrintStructure(currentNode->children[i], depth + 1);
            }
        }
    }
};

template<typename TKey, typename TElement>
TElement& BTree<TKey, TElement>::operator[](const TKey &key) {
    if (!root) {
        root = ShrdPtr<Node>(new Node(true, order));
    }

    ShrdPtr<Node> node = root;
    while (node) {
        int index = 0;
        while (index < node->numKeys && key > node->keys[index])
            ++index;

        if (index < node->numKeys && key == node->keys[index]) {
            return node->values[index];
        }

        if (node->isLeaf) {
            if (node->numKeys >= 2 * order - 1) {
                throw std::runtime_error("BTree node is full, cannot insert.");
            }
            // Вставляем новый ключ в листовой узел
            for (int j = node->numKeys; j > index; --j) {
                node->keys[j] = node->keys[j - 1];
                node->values[j] = node->values[j - 1];
            }
            node->keys[index] = key;
            node->values[index] = TElement(); // Создание нового значения по умолчанию
            node->numKeys++;
            return node->values[index];
        }

        node = node->children[index];
    }

    throw std::runtime_error("Unexpected error in BTree operator[].");
}

template<typename TKey, typename TElement>
BTree<TKey, TElement>::Node::Node(bool leaf, int order)
        : isLeaf(leaf), numKeys(0), keys(new TKey[2 * order - 1]), values(new TElement[2 * order - 1]),
          children(new ShrdPtr<Node>[2 * order]) {
}

template<typename TKey, typename TElement>
BTree<TKey, TElement>::BTree(int order)
        : root(new Node(true, order)), order(order), count(0) {
}

template<typename TKey, typename TElement>
BTree<TKey, TElement>::~BTree() {}

template<typename TKey, typename TElement>
size_t BTree<TKey, TElement>::GetCount() const {
    return count;
}

template<typename TKey, typename TElement>
void BTree<TKey, TElement>::Add(const TKey &key, const TElement &element) {
    if (ContainsKey(key)) {
        (*this)[key] = element;
        return;
    }

    if (root->numKeys == 2 * order - 1) {
        ShrdPtr<Node> newRoot(new Node(false, order));
        newRoot->children[0] = root;
        SplitChild(newRoot, 0);
        root = newRoot;
    }
    InsertNonFull(root, key, element);
    ++count;
}

template<typename TKey, typename TElement>
void BTree<TKey, TElement>::InsertNonFull(ShrdPtr<Node> &node, const TKey &key, const TElement &value) {
    int i = node->numKeys - 1;

    if (node->isLeaf) {
        while (i >= 0 && key < node->keys[i]) {
            node->keys[i + 1] = node->keys[i];
            node->values[i + 1] = node->values[i];
            --i;
        }
        node->keys[i + 1] = key;
        node->values[i + 1] = value;
        ++node->numKeys;
    } else {
        while (i >= 0 && key < node->keys[i])
            --i;
        ++i;
        if (node->children[i]->numKeys == 2 * order - 1) {
            SplitChild(node, i);
            if (key > node->keys[i])
                ++i;
        }
        InsertNonFull(node->children[i], key, value);
    }
}

template<typename TKey, typename TElement>
void BTree<TKey, TElement>::SplitChild(ShrdPtr<Node> parentNode, int childIndex) {
    ShrdPtr<Node> oldChild = parentNode->children[childIndex];
    ShrdPtr<Node> newChild(new Node(oldChild->isLeaf, order));

    newChild->numKeys = order - 1;

    for (int i = 0; i < order - 1; ++i) {
        newChild->keys[i] = oldChild->keys[i + order];
        newChild->values[i] = oldChild->values[i + order];
    }

    if (!oldChild->isLeaf) {
        for (int i = 0; i < order; ++i) {
            newChild->children[i] = oldChild->children[i + order];
        }
    }

    oldChild->numKeys = order - 1;

    for (int i = parentNode->numKeys; i >= childIndex + 1; --i) {
        parentNode->children[i + 1] = parentNode->children[i];
    }
    parentNode->children[childIndex + 1] = newChild;

    for (int i = parentNode->numKeys - 1; i >= childIndex; --i) {
        parentNode->keys[i + 1] = parentNode->keys[i];
        parentNode->values[i + 1] = parentNode->values[i];
    }

    parentNode->keys[childIndex] = oldChild->keys[order - 1];
    parentNode->values[childIndex] = oldChild->values[order - 1];
    ++parentNode->numKeys;
}

template<typename TKey, typename TElement>
TElement BTree<TKey, TElement>::Get(const TKey &key) const {
    ShrdPtr<Node> node = root;
    while (node) {
        int i = 0;
        while (i < node->numKeys && key > node->keys[i])
            ++i;

        if (i < node->numKeys && key == node->keys[i])
            return node->values[i];

        if (node->isLeaf)
            throw std::runtime_error("Key not found.");

        node = node->children[i];
    }
    throw std::runtime_error("Key not found.");
}

template<typename TKey, typename TElement>
TElement BTree<TKey, TElement>::Search(ShrdPtr<Node> node, const TKey &key) const {
    int index = 0;
    while (index < node->numKeys && key > node->keys[index])
        ++index;

    if (index < node->numKeys && key == node->keys[index])
        return node->values[index];

    if (node->isLeaf)
        throw std::runtime_error("Key not found.");
    else
        return Search(node->children[index], key);
}

template<typename TKey, typename TElement>
bool BTree<TKey, TElement>::ContainsKey(const TKey &key) const {
    try {
        Get(key);
        return true;
    } catch (...) {
        return false;
    }
}

template<typename TKey, typename TElement>
void BTree<TKey, TElement>::Remove(const TKey &key) {
    if (!ContainsKey(key))
        throw std::runtime_error("Key not found.");

    RemoveFromNode(root, key);
    --count;

    if (root->numKeys == 0) {
        if (root->isLeaf) {
            root.reset(new Node(true, order));
        } else {
            root = root->children[0];
        }
    }
}

template<typename TKey, typename TElement>
void BTree<TKey, TElement>::RemoveFromNode(ShrdPtr<Node> node, const TKey &key) {
    int index = 0;
    while (index < node->numKeys && node->keys[index] < key)
        ++index;

    if (index < node->numKeys && node->keys[index] == key) {
        if (node->isLeaf)
            RemoveFromLeaf(node, index);
        else
            RemoveFromNonLeaf(node, index);
    } else if (!node->isLeaf) {
        Fill(node, index);
        if (index < node->numKeys)
            RemoveFromNode(node->children[index], key);
        else
            RemoveFromNode(node->children[index - 1], key);
    }
}

template<typename TKey, typename TElement>
void BTree<TKey, TElement>::RemoveFromLeaf(ShrdPtr<Node> node, int idx) {
    for (int i = idx; i < node->numKeys - 1; ++i) {
        node->keys[i] = node->keys[i + 1];
        node->values[i] = node->values[i + 1];
    }
    --node->numKeys;
}

template<typename TKey, typename TElement>
void BTree<TKey, TElement>::RemoveFromNonLeaf(ShrdPtr<Node> node, int idx) {
    TKey key = node->keys[idx];
    if (node->children[idx]->numKeys >= order) {
        TKey predecessor = GetPredecessor(node, idx);
        node->keys[idx] = predecessor;
        RemoveFromNode(node->children[idx], predecessor);
    } else if (node->children[idx + 1]->numKeys >= order) {
        TKey successor = GetSuccessor(node, idx);
        node->keys[idx] = successor;
        RemoveFromNode(node->children[idx + 1], successor);
    } else {
        Merge(node, idx);
        RemoveFromNode(node->children[idx], key);
    }
}

template<typename TKey, typename TElement>
TKey BTree<TKey, TElement>::GetPredecessor(ShrdPtr<Node> node, int idx) {
    ShrdPtr<Node> current = node->children[idx];
    while (!current->isLeaf)
        current = current->children[current->numKeys];
    return current->keys[current->numKeys - 1];
}

template<typename TKey, typename TElement>
TKey BTree<TKey, TElement>::GetSuccessor(ShrdPtr<Node> node, int idx) {
    ShrdPtr<Node> current = node->children[idx + 1];
    while (!current->isLeaf)
        current = current->children[0];
    return current->keys[0];
}

template<typename TKey, typename TElement>
void BTree<TKey, TElement>::Fill(ShrdPtr<Node> node, int idx) {
    if (idx > 0 && node->children[idx - 1]->numKeys >= order)
        BorrowFromPrev(node, idx);
    else if (idx < node->numKeys && node->children[idx + 1]->numKeys >= order)
        BorrowFromNext(node, idx);
    else {
        if (idx < node->numKeys)
            Merge(node, idx);
        else
            Merge(node, idx - 1);
    }
}

template<typename TKey, typename TElement>
void BTree<TKey, TElement>::BorrowFromPrev(ShrdPtr<Node> node, int idx) {
    ShrdPtr<Node> child = node->children[idx];
    ShrdPtr<Node> sibling = node->children[idx - 1];

    for (int i = child->numKeys - 1; i >= 0; --i) {
        child->keys[i + 1] = child->keys[i];
        child->values[i + 1] = child->values[i];
    }

    if (!child->isLeaf) {
        for (int i = child->numKeys; i >= 0; --i)
            child->children[i + 1] = child->children[i];
    }

    child->keys[0] = node->keys[idx - 1];
    child->values[0] = node->values[idx - 1];

    if (!child->isLeaf)
        child->children[0] = sibling->children[sibling->numKeys];

    node->keys[idx - 1] = sibling->keys[sibling->numKeys - 1];
    node->values[idx - 1] = sibling->values[sibling->numKeys - 1];

    ++child->numKeys;
    --sibling->numKeys;
}

template<typename TKey, typename TElement>
void BTree<TKey, TElement>::BorrowFromNext(ShrdPtr<Node> node, int idx) {
    ShrdPtr<Node> child = node->children[idx];
    ShrdPtr<Node> sibling = node->children[idx + 1];

    child->keys[child->numKeys] = node->keys[idx];
    child->values[child->numKeys] = node->values[idx];

    if (!child->isLeaf)
        child->children[child->numKeys + 1] = sibling->children[0];

    node->keys[idx] = sibling->keys[0];
    node->values[idx] = sibling->values[0];

    for (int i = 1; i < sibling->numKeys; ++i) {
        sibling->keys[i - 1] = sibling->keys[i];
        sibling->values[i - 1] = sibling->values[i];
    }

    if (!sibling->isLeaf) {
        for (int i = 1; i <= sibling->numKeys; ++i)
            sibling->children[i - 1] = sibling->children[i];
    }

    ++child->numKeys;
    --sibling->numKeys;
}

template<typename TKey, typename TElement>
void BTree<TKey, TElement>::Merge(ShrdPtr<Node> node, int idx) {
    ShrdPtr<Node> child = node->children[idx];
    ShrdPtr<Node> sibling = node->children[idx + 1];

    child->keys[order - 1] = node->keys[idx];
    child->values[order - 1] = node->values[idx];

    for (int i = 0; i < sibling->numKeys; ++i) {
        child->keys[i + order] = sibling->keys[i];
        child->values[i + order] = sibling->values[i];
    }

    if (!child->isLeaf) {
        for (int i = 0; i <= sibling->numKeys; ++i)
            child->children[i + order] = sibling->children[i];
    }

    for (int i = idx + 1; i < node->numKeys; ++i) {
        node->keys[i - 1] = node->keys[i];
        node->values[i - 1] = node->values[i];
    }

    for (int i = idx + 2; i <= node->numKeys; ++i)
        node->children[i - 1] = node->children[i];

    --node->numKeys;
    --sibling->numKeys;
}
template<typename TKey, typename TElement>
BTree<TKey, TElement>::BTreeIterator::BTreeIterator(const BTree *tree)
        : tree(tree), hasCurrent(false) {
    Reset();
}

template<typename TKey, typename TElement>
void BTree<TKey, TElement>::BTreeIterator::Reset() {
    stack = DynamicArraySmart<StackNode>();
    hasCurrent = false;
    if (tree->root) {
        PushLeftmost(tree->root);
    }
}

template<typename TKey, typename TElement>
void BTree<TKey, TElement>::BTreeIterator::PushLeftmost(ShrdPtr<Node> node) {
    while (node && node->numKeys > 0) {
        StackNode sn = {node, 0};
        stack.Append(sn);
        if (node->isLeaf)
            break;
        else
            node = node->children[0];
    }
}

template<typename TKey, typename TElement>
bool BTree<TKey, TElement>::BTreeIterator::MoveNext() {
    while (stack.GetLength() > 0) {
        StackNode &top = stack[stack.GetLength() - 1];

        if (top.index < top.node->numKeys) {
            if (top.node->numKeys == 0) {
                stack.RemoveAt(stack.GetLength() - 1);
                continue;
            }

            currentKey = top.node->keys[top.index];
            currentValue = top.node->values[top.index];
            hasCurrent = true;

            if (!top.node->isLeaf) {
                if (top.index + 1 <= top.node->numKeys) {
                    ShrdPtr<Node> child = top.node->children[top.index + 1];
                    ++top.index;
                    PushLeftmost(child);
                } else {
                    ++top.index;
                }
            } else {
                ++top.index;
            }

            return true;
        } else {
            stack.RemoveAt(stack.GetLength() - 1);
        }
    }

    hasCurrent = false;
    return false;
}


template<typename TKey, typename TElement>
TKey BTree<TKey, TElement>::BTreeIterator::GetCurrentKey() const {
    if (!hasCurrent)
        throw std::out_of_range("Iterator out of range");
    return currentKey;
}

template<typename TKey, typename TElement>
TElement BTree<TKey, TElement>::BTreeIterator::GetCurrentValue() const {
    if (!hasCurrent)
        throw std::out_of_range("Iterator out of range");
    return currentValue;
}


template<typename TKey, typename TElement>
UnqPtr<IDictionaryIterator<TKey, TElement>> BTree<TKey, TElement>::GetIterator() const {
    return UnqPtr<IDictionaryIterator<TKey, TElement>>(new BTreeIterator(this));
}

template<typename TKey, typename TElement>
TElement& BTree<TKey, TElement>::FindOrInsert(ShrdPtr<Node> &node, const TKey &key) {
    int i = 0;
    while (i < node->numKeys && key > node->keys[i])
        ++i;

    if (i < node->numKeys && key == node->keys[i]) {
        return node->values[i];
    }

    if (node->isLeaf) {
        if (node->numKeys == 2 * order - 1) {
            throw std::runtime_error("BTree node is full, cannot insert.");
        }
        node->keys[node->numKeys] = key;
        node->values[node->numKeys] = TElement();
        ++node->numKeys;
        return node->values[node->numKeys - 1];
    }

    return FindOrInsert(node->children[i], key);
}

#endif // BTREE_H
