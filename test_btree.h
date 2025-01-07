#ifndef TEST_BTREE_H
#define TEST_BTREE_H

#include "DifferentStructures/BTree.h"
#include "DifferentStructures/UnqPtr.h"
#include "DifferentStructures/ShrdPtr.h"
#include "DifferentStructures/DynamicArraySmart.h"
#include <string>
#include <vector>
#include <random>
#include <iostream>
#include <fstream>

class BTreeTest {
public:
    BTreeTest(int order = 3);

    void AddElement(int key, const std::string &value);

    void RemoveElement(int key);

    void FillRandom(int numElements, int keyMin, int keyMax);

    void Visualize(const std::string &filename = "btree.png");

    void DisplayTree();

    bool CheckBalance();

private:
    BTree<int, std::string> btree;

    void
    Traverse(const BTree<int, std::string>::Node *node, int &nodeCount, std::ostream &out, int parent, int childIndex,
             int &nullCount) const;

    int GetDepth(const BTree<int, std::string>::Node *node) const;
};

#endif // TEST_BTREE_H
