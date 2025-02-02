#include "menu.h"
#include <iostream>
using namespace std;
#include "DifferentStructures/SparseVector.h"
#include "DifferentStructures/SparseMatrix.h"
#include "DifferentStructures/HashTable.h"
#include "DifferentStructures/BTree.h"
#include "DifferentStructures/UnqPtr.h"
#include <cmath>

void displayMenu() {
    int structureChoice = 0;
    int dictionaryChoice = 0;

    std::cout << "=== Select Structure ===\n";
    std::cout << "1. Sparse Vector\n";
    std::cout << "2. Sparse Matrix\n";
    std::cout << "Your choice: ";
    std::cin >> structureChoice;

    std::cout << "\n=== Select Dictionary ===\n";
    std::cout << "1. HashTable\n";
    std::cout << "2. BTree\n";
    std::cout << "Your choice: ";
    std::cin >> dictionaryChoice;

    if (structureChoice == 1) {
        int length = 30;
        UnqPtr<IDictionary<int, double>> dictionary;

        if (dictionaryChoice == 1) {
            dictionary = UnqPtr<IDictionary<int, double>>(new HashTable<int, double>());
            std::cout << "\n[INFO] Using HashTable for Sparse Vector.\n";
        } else if (dictionaryChoice == 2) {
            dictionary = UnqPtr<IDictionary<int, double>>(new BTree<int, double>());
            std::cout << "\n[INFO] Using BTree for Sparse Vector.\n";
        } else {
            std::cerr << "Error: Invalid dictionary choice.\n";
            return;
        }

        auto sparseVector = UnqPtr<SparseVector<double>>(new SparseVector<double>(length, std::move(dictionary)));
        handleVectorOperations(sparseVector);

    } else if (structureChoice == 2) {
        int rows = 5, cols = 5;
        UnqPtr<IDictionary<IndexPair, double>> dictionary;

        if (dictionaryChoice == 1) {
            dictionary = UnqPtr<IDictionary<IndexPair, double>>(new HashTable<IndexPair, double>());
            std::cout << "\n[INFO] Using HashTable for Sparse Matrix.\n";
        } else if (dictionaryChoice == 2) {
            dictionary = UnqPtr<IDictionary<IndexPair, double>>(new BTree<IndexPair, double>());
            std::cout << "\n[INFO] Using BTree for Sparse Matrix.\n";
        } else {
            std::cerr << "Error: Invalid dictionary choice.\n";
            return;
        }

        auto sparseMatrix = UnqPtr<SparseMatrix<double>>(new SparseMatrix<double>(rows, cols));
        handleMatrixOperations(sparseMatrix); // передаем через UnqPtr
    } else {
        std::cerr << "Error: Invalid structure choice.\n";
    }
}

void handleVectorOperations(UnqPtr<SparseVector<double>> &sparseVector) {
    while (true) {
        int choice;
        std::cout << "\n=== Sparse Vector Operations ===\n";
        std::cout << "1. Set Element\n";
        std::cout << "2. Get Element\n";
        std::cout << "3. Display Elements\n";
        std::cout << "4. Multiply by Scalar\n";
        std::cout << "5. Exit\n";
        std::cout << "Your choice: ";
        std::cin >> choice;

        if (choice == 1) {
            int index;
            double value;
            std::cout << "Enter index (0-" << sparseVector->GetLength() - 1 << "): ";
            std::cin >> index;
            std::cout << "Enter value: ";
            std::cin >> value;

            sparseVector->SetElement(index, value);
        } else if (choice == 2) {
            int index;
            std::cout << "Enter index: ";
            std::cin >> index;
            std::cout << "Value: " << sparseVector->GetElement(index) << "\n";
        } else if (choice == 3) {
            updateVectorDisplay(*sparseVector);
        } else if (choice == 4) {
            double scalar;
            std::cout << "Enter scalar: ";
            std::cin >> scalar;
            sparseVector->MultiplyByScalar(scalar);
            std::cout << "Vector multiplied successfully.\n";
        } else if (choice == 5) {
            break;
        }
    }
}

void handleMatrixOperations(UnqPtr<SparseMatrix<double>> &sparseMatrix) {
    while (true) {
        int choice;
        std::cout << "\n=== Sparse Matrix Operations ===\n";
        std::cout << "1. Set Element\n";
        std::cout << "2. Get Element\n";
        std::cout << "3. Display Elements\n";
        std::cout << "4. Multiply by Scalar\n";
        std::cout << "5. Exit\n";
        std::cout << "Your choice: ";
        std::cin >> choice;

        if (choice == 1) {
            int row, col;
            double value;
            std::cout << "Enter row: ";
            std::cin >> row;
            std::cout << "Enter column: ";
            std::cin >> col;
            std::cout << "Enter value: ";
            std::cin >> value;
            sparseMatrix->SetElement(row, col, value);
        } else if (choice == 2) {
            int row, col;
            std::cout << "Enter row: ";
            std::cin >> row;
            std::cout << "Enter column: ";
            std::cin >> col;
            std::cout << "Value: " << sparseMatrix->GetElement(row, col) << "\n";
        } else if (choice == 3) {
            updateMatrixDisplay(*sparseMatrix);
        } else if (choice == 4) {
            double scalar;
            std::cout << "Enter scalar: ";
            std::cin >> scalar;
            sparseMatrix->MultiplyByScalar(scalar);
            std::cout << "Matrix multiplied successfully.\n";
        } else if (choice == 5) {
            break;
        }
    }
}

void updateVectorDisplay(const SparseVector<double> &sparseVector) {
    std::cout << "\nSparse Vector Elements:\n";
    for (int i = 0; i < sparseVector.GetLength(); ++i) {
        try {
            double value = sparseVector.GetElement(i);
            std::cout << "Index " << i << ": " << value << "\n";
        } catch (const std::exception &) {
            // Skip missing elements
        }
    }
}

void updateMatrixDisplay(const SparseMatrix<double> &sparseMatrix) {
    std::cout << "\nSparse Matrix Elements:\n";
    for (int i = 0; i < sparseMatrix.GetRows(); ++i) {
        for (int j = 0; j < sparseMatrix.GetColumns(); ++j) {
            try {
                double value = sparseMatrix.GetElement(i, j);
                std::cout << value << "\t";
            } catch (const std::exception &) {
                std::cout << "0\t"; // Empty cells
            }
        }
        std::cout << "\n";
    }
}
