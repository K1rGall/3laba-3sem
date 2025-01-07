#ifndef TEST_SPARSE_VECTOR_H
#define TEST_SPARSE_VECTOR_H

#include "DifferentStructures/SparseVector.h"
#include "DifferentStructures/HashTable.h"
#include "DifferentStructures/BTree.h"
#include <iostream>
#include <chrono>

inline void test_sparse_vector() {
    std::cout << "=== Testing SparseVector ===\n";

    {
        std::cout << "  [Test] Using HashTable as storage...\n";
        UnqPtr<IDictionary<int, double>> dictionary(new HashTable<int, double>());
        SparseVector<double> vector(6, std::move(dictionary));

        vector.SetElement(0, 2.5);
        vector.SetElement(3, -4.8);
        vector.SetElement(5, 7.1);

        std::cout << "  SparseVector with HashTable:\n";
        for (int i = 0; i < 6; ++i) {
            std::cout << "    Element[" << i << "] = " << vector.GetElement(i) << "\n";
        }
    }

    {
        std::cout << "  [Test] Using BTree as storage...\n";
        UnqPtr<IDictionary<int, double>> dictionary(new BTree<int, double>());
        SparseVector<double> vector(6, std::move(dictionary));

        vector.SetElement(1, -3.2);
        vector.SetElement(4, 6.4);
        vector.SetElement(5, 1.9);

        std::cout << "  SparseVector with BTree:\n";
        for (int i = 0; i < 6; ++i) {
            std::cout << "    Element[" << i << "] = " << vector.GetElement(i) << "\n";
        }
    }

    std::cout << "=== SparseVector Tests Completed ===\n";
}

inline std::pair<double, double> load_test_sparse_vector(const int& size, const int& numElements) {
    if (size <= 0) {
        std::cerr << "Error: Vector size must be positive.\n";
        return {};
    }
    if (numElements < 0) {
        std::cerr << "Error: Number of elements must be non-negative.\n";
        return {};
    }

    std::pair<double, double> hashTableBTreeTime;

    {
        std::cout << "  [Benchmark] Testing SparseVector with HashTable...\n";
        UnqPtr<IDictionary<int, double>> dictionary(new HashTable<int, double>());
        SparseVector<double> vector(size, std::move(dictionary));

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < numElements; ++i) {
            int index = (i * 37) % size;  // Altered hash distribution for better spread
            vector.SetElement(index, static_cast<double>(i * 1.5));
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        hashTableBTreeTime.first = duration.count();

        std::cout << "  Time taken with HashTable: " << hashTableBTreeTime.first << " seconds.\n";
    }

    {
        std::cout << "  [Benchmark] Testing SparseVector with BTree...\n";
        UnqPtr<IDictionary<int, double>> dictionary(new BTree<int, double>());
        SparseVector<double> vector(size, std::move(dictionary));

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < numElements; ++i) {
            int index = (i * 37) % size;
            vector.SetElement(index, static_cast<double>(i * 1.5));
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        hashTableBTreeTime.second = duration.count();

        std::cout << "  Time taken with BTree: " << hashTableBTreeTime.second << " seconds.\n";
    }

    return hashTableBTreeTime;
}

#endif // TEST_SPARSE_VECTOR_H
