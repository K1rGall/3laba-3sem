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

    virtual size_t GetCapacity() const override;

    virtual TElement Get(const TKey &key) const override;

    virtual bool ContainsKey(const TKey &key) const override;

    virtual void Add(const TKey &key, const TElement &element) override;

    virtual void Remove(const TKey &key) override;

    virtual void Update(const TKey &key, const TElement &element) override;

    virtual UnqPtr<IDictionaryIterator<TKey, TElement>> GetIterator() const override;

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

    void SplitChild(ShrdPtr<Node> x, int i);

    void InsertNonFull(ShrdPtr<Node> x, const TKey &key, const TElement &value);

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
BTree<TKey, TElement>::Node::Node(bool leaf, int order)
        : isLeaf(leaf), numKeys(0), keys(new TKey[2 * order - 1]), values(new TElement[2 * order - 1]),
          children(new ShrdPtr<Node>[2 * order]) {
}

template<typename TKey, typename TElement>
BTree<TKey, TElement>::BTree(int order)
        : root(new Node(true, order)), order(order), count(0) {
}

template<typename TKey, typename TElement>
BTree<TKey, TElement>::~BTree() {
}

template<typename TKey, typename TElement>
size_t BTree<TKey, TElement>::GetCount() const {
    return count;
}

template<typename TKey, typename TElement>
size_t BTree<TKey, TElement>::GetCapacity() const {
    return count;
}

template<typename TKey, typename TElement>
void BTree<TKey, TElement>::Add(const TKey &key, const TElement &element) {
    if (ContainsKey(key)) {
        Update(key, element);
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
void BTree<TKey, TElement>::InsertNonFull(ShrdPtr<Node> node, const TKey &key, const TElement &value) {
    int index = node->numKeys - 1;

    if (node->isLeaf) {
        while (index >= 0 && key < node->keys[index]) {
            node->keys[index + 1] = node->keys[index];
            node->values[index + 1] = node->values[index];
            --index;
        }
        node->keys[index + 1] = key;
        node->values[index + 1] = value;
        ++node->numKeys;
    } else {
        while (index >= 0 && key < node->keys[index])
            --index;
        ++index;
        if (node->children[index]->numKeys == 2 * order - 1) {
            SplitChild(node, index);
            if (key > node->keys[index])
                ++index;
        }
        InsertNonFull(node->children[index], key, value);
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
        for (int i = 0; i < order; ++i)
            newChild->children[i] = oldChild->children[i + order];
    }

    oldChild->numKeys = order - 1;

    for (int i = parentNode->numKeys; i >= childIndex + 1; --i)
        parentNode->children[i + 1] = parentNode->children[i];
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
    return Search(root, key);
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
        Search(root, key);
        return true;
    }
    catch (const std::runtime_error &) {
        return false;
    }
}

template<typename TKey, typename TElement>
void BTree<TKey, TElement>::Update(const TKey &key, const TElement &element) {
    ShrdPtr<Node> node = root;
    while (true) {
        int index = 0;
        while (index < node->numKeys && key > node->keys[index])
            ++index;

        if (index < node->numKeys && key == node->keys[index]) {
            node->values[index] = element;
            return;
        }

        if (node->isLeaf)
            throw std::runtime_error("Key not found.");

        node = node->children[index];
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
    } else {
        if (node->isLeaf)
            throw std::runtime_error("Key not found.");

        bool flag = ((index == node->numKeys));

        if (node->children[index]->numKeys < order)
            Fill(node, index);

        if (flag && index > node->numKeys)
            RemoveFromNode(node->children[index - 1], key);
        else
            RemoveFromNode(node->children[index], key);
    }
}

template<typename TKey, typename TElement>
void BTree<TKey, TElement>::RemoveFromLeaf(ShrdPtr<Node> node, int index) {
    for (int i = index + 1; i < node->numKeys; ++i) {
        node->keys[i - 1] = node->keys[i];
        node->values[i - 1] = node->values[i];
    }
    --node->numKeys;
}

template<typename TKey, typename TElement>
void BTree<TKey, TElement>::RemoveFromNonLeaf(ShrdPtr<Node> node, int index) {
    TKey key = node->keys[index];

    if (node->children[index]->numKeys >= order) {
        TKey predKey = GetPredecessor(node, index);
        TElement predValue = node->values[index];
        node->keys[index] = predKey;
        node->values[index] = predValue;
        RemoveFromNode(node->children[index], predKey);
    } else if (node->children[index + 1]->numKeys >= order) {
        TKey succKey = GetSuccessor(node, index);
        TElement succValue = node->values[index];
        node->keys[index] = succKey;
        node->values[index] = succValue;
        RemoveFromNode(node->children[index + 1], succKey);
    } else {
        Merge(node, index);
        RemoveFromNode(node->children[index], key);
    }
}

template<typename TKey, typename TElement>
TKey BTree<TKey, TElement>::GetPredecessor(ShrdPtr<Node> node, int index) {
    ShrdPtr<Node> currentNode = node->children[index];
    while (!currentNode->isLeaf)
        currentNode = currentNode->children[currentNode->numKeys];
    return currentNode->keys[currentNode->numKeys - 1];
}

template<typename TKey, typename TElement>
TKey BTree<TKey, TElement>::GetSuccessor(ShrdPtr<Node> node, int index) {
    ShrdPtr<Node> currentNode = node->children[index + 1];
    while (!currentNode->isLeaf)
        currentNode = currentNode->children[0];
    return currentNode->keys[0];
}

template<typename TKey, typename TElement>
void BTree<TKey, TElement>::Fill(ShrdPtr<Node> node, int index) {
    if (index != 0 && node->children[index - 1]->numKeys >= order)
        BorrowFromPrev(node, index);
    else if (index != node->numKeys && node->children[index + 1]->numKeys >= order)
        BorrowFromNext(node, index);
    else {
        if (index != node->numKeys)
            Merge(node, index);
        else
            Merge(node, index - 1);
    }
}

template<typename TKey, typename TElement>
void BTree<TKey, TElement>::BorrowFromPrev(ShrdPtr<Node> node, int index) {
    ShrdPtr<Node> child = node->children[index];
    ShrdPtr<Node> sibling = node->children[index - 1];

    for (int i = child->numKeys - 1; i >= 0; --i) {
        child->keys[i + 1] = child->keys[i];
        child->values[i + 1] = child->values[i];
    }

    if (!child->isLeaf) {
        for (int i = child->numKeys; i >= 0; --i)
            child->children[i + 1] = child->children[i];
    }

    child->keys[0] = node->keys[index - 1];
    child->values[0] = node->values[index - 1];

    if (!child->isLeaf)
        child->children[0] = sibling->children[sibling->numKeys];

    node->keys[index - 1] = sibling->keys[sibling->numKeys - 1];
    node->values[index - 1] = sibling->values[sibling->numKeys - 1];

    --sibling->numKeys;
    ++child->numKeys;
}

template<typename TKey, typename TElement>
void BTree<TKey, TElement>::BorrowFromNext(ShrdPtr<Node> node, int index) {
    ShrdPtr<Node> child = node->children[index];
    ShrdPtr<Node> sibling = node->children[index + 1];

    child->keys[child->numKeys] = node->keys[index];
    child->values[child->numKeys] = node->values[index];

    if (!child->isLeaf)
        child->children[child->numKeys + 1] = sibling->children[0];

    node->keys[index] = sibling->keys[0];
    node->values[index] = sibling->values[0];

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
void BTree<TKey, TElement>::Merge(ShrdPtr<Node> node, int index) {
    ShrdPtr<Node> child = node->children[index];
    ShrdPtr<Node> sibling = node->children[index + 1];

    child->keys[order - 1] = node->keys[index];
    child->values[order - 1] = node->values[index];

    for (int i = 0; i < sibling->numKeys; ++i) {
        child->keys[i + order] = sibling->keys[i];
        child->values[i + order] = sibling->values[i];
    }

    if (!child->isLeaf) {
        for (int i = 0; i <= sibling->numKeys; ++i)
            child->children[i + order] = sibling->children[i];
    }

    for (int i = index + 1; i < node->numKeys; ++i) {
        node->keys[i - 1] = node->keys[i];
        node->values[i - 1] = node->values[i];
    }

    for (int i = index + 2; i <= node->numKeys; ++i)
        node->children[i - 1] = node->children[i];

    --node->numKeys;
    --sibling->numKeys;
    ++child->numKeys;
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

#endif // BTREE_H
