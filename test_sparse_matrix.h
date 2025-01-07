#ifndef TEST_SPARSE_MATRIX_H
#define TEST_SPARSE_MATRIX_H

#include "DifferentStructures/SparseMatrix.h"
#include "DifferentStructures/HashTable.h"
#include "DifferentStructures/BTree.h"
#include <iostream>
#include <chrono>

inline void test_sparse_matrix() {
    std::cout << "=== Testing SparseMatrix ===\n";

    {
        std::cout << "  [Test] Using HashTable as storage...\n";
        UnqPtr<IDictionary<IndexPair, double>> dictionary(new HashTable<IndexPair, double>());
        SparseMatrix<double> matrix(4, 4, std::move(dictionary));

        matrix.SetElement(0, 1, 5.5);
        matrix.SetElement(2, 3, 10.1);
        matrix.SetElement(3, 0, 7.3);

        std::cout << "  SparseMatrix with HashTable:\n";
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                std::cout << "    Element[" << i << "][" << j << "] = "
                          << matrix.GetElement(i, j) << "\n";
            }
        }
    }

    {
        std::cout << "  [Test] Using BTree as storage...\n";
        UnqPtr<IDictionary<IndexPair, double>> dictionary(new BTree<IndexPair, double>());
        SparseMatrix<double> matrix(4, 4, std::move(dictionary));

        matrix.SetElement(0, 2, 8.8);
        matrix.SetElement(1, 1, 6.6);
        matrix.SetElement(3, 3, 4.4);

        std::cout << "  SparseMatrix with BTree:\n";
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                std::cout << "    Element[" << i << "][" << j << "] = "
                          << matrix.GetElement(i, j) << "\n";
            }
        }
    }

    std::cout << "=== SparseMatrix Tests Completed ===\n";
}

inline std::pair<double, double> load_test_sparse_matrix(const int& size, const int& numElements) {
    if (size <= 0) {
        std::cerr << "Error: Matrix size must be positive.\n";
        return {};
    }
    if (numElements < 0) {
        std::cerr << "Error: Number of elements must be non-negative.\n";
        return {};
    }

    std::pair<double, double> hashTableBTreeTime;

    {
        std::cout << "  [Benchmark] Testing SparseMatrix with HashTable...\n";
        UnqPtr<IDictionary<IndexPair, double>> dictionary(new HashTable<IndexPair, double>());
        SparseMatrix<double> matrix(size, size, std::move(dictionary));

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < numElements; ++i) {
            int row = i % size;
            int col = (i * 37) % size;  // Slightly altered hash distribution
            matrix.SetElement(row, col, static_cast<double>(i * 0.1));
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        hashTableBTreeTime.first = duration.count();

        std::cout << "  Time taken with HashTable: " << hashTableBTreeTime.first << " seconds.\n";
    }

    {
        std::cout << "  [Benchmark] Testing SparseMatrix with BTree...\n";
        UnqPtr<IDictionary<IndexPair, double>> dictionary(new BTree<IndexPair, double>());
        SparseMatrix<double> matrix(size, size, std::move(dictionary));

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < numElements; ++i) {
            int row = i % size;
            int col = (i * 37) % size;  // Matching distribution for comparability
            matrix.SetElement(row, col, static_cast<double>(i * 0.1));
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        hashTableBTreeTime.second = duration.count();

        std::cout << "  Time taken with BTree: " << hashTableBTreeTime.second << " seconds.\n";
    }

    return hashTableBTreeTime;
}

#endif // TEST_SPARSE_MATRIX_H
