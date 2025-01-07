#include "menu.h"

#include <iostream>
#include "DifferentStructures/SparseVector.h"
#include "DifferentStructures/SparseMatrix.h"
#include "DifferentStructures/HashTable.h"
#include "DifferentStructures/BTree.h"
#include "DifferentStructures/UnqPtr.h"
#include <cmath>

// Меню выбора структуры и словаря
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

        auto sparseMatrix = UnqPtr<SparseMatrix<double>>(new SparseMatrix<double>(rows, cols, std::move(dictionary)));
        handleMatrixOperations(sparseMatrix);

    } else {
        std::cerr << "Error: Invalid structure choice.\n";
    }
}

// Операции для SparseVector
void handleVectorOperations(UnqPtr<SparseVector<double>> &sparseVector) {
    while (true) {
        int choice;
        std::cout << "\n=== Sparse Vector Operations ===\n";
        std::cout << "1. Set Element\n";
        std::cout << "2. Get Element\n";
        std::cout << "3. Display Elements\n";
        std::cout << "4. Exit\n";
        std::cout << "Your choice: ";
        std::cin >> choice;

        if (choice == 1) {
            int index;
            double value;
            std::cout << "Enter index (0-" << sparseVector->GetLength() - 1 << "): ";
            std::cin >> index;
            std::cout << "Enter value: ";
            std::cin >> value;

            try {
                sparseVector->SetElement(index, value);
                std::cout << "[INFO] Element at index " << index << " set to " << value << ".\n";
            } catch (const std::exception &e) {
                std::cerr << "Error: " << e.what() << "\n";
            }

        } else if (choice == 2) {
            int index;
            std::cout << "Enter index (0-" << sparseVector->GetLength() - 1 << "): ";
            std::cin >> index;

            try {
                double value = sparseVector->GetElement(index);
                std::cout << "[INFO] Value at index " << index << ": " << value << "\n";
            } catch (const std::exception &e) {
                std::cerr << "Error: " << e.what() << "\n";
            }

        } else if (choice == 3) {
            updateVectorDisplay(*sparseVector);
        } else if (choice == 4) {
            break;
        } else {
            std::cout << "Invalid choice. Try again.\n";
        }
    }
}

// Операции для SparseMatrix
void handleMatrixOperations(UnqPtr<SparseMatrix<double>> &sparseMatrix) {
    while (true) {
        int choice;
        std::cout << "\n=== Sparse Matrix Operations ===\n";
        std::cout << "1. Set Element\n";
        std::cout << "2. Get Element\n";
        std::cout << "3. Display Elements\n";
        std::cout << "4. Exit\n";
        std::cout << "Your choice: ";
        std::cin >> choice;

        if (choice == 1) {
            int row, col;
            double value;
            std::cout << "Enter row (0-" << sparseMatrix->GetRows() - 1 << "): ";
            std::cin >> row;
            std::cout << "Enter column (0-" << sparseMatrix->GetColumns() - 1 << "): ";
            std::cin >> col;
            std::cout << "Enter value: ";
            std::cin >> value;

            try {
                sparseMatrix->SetElement(row, col, value);
                std::cout << "[INFO] Element at (" << row << ", " << col << ") set to " << value << ".\n";
            } catch (const std::exception &e) {
                std::cerr << "Error: " << e.what() << "\n";
            }

        } else if (choice == 2) {
            int row, col;
            std::cout << "Enter row (0-" << sparseMatrix->GetRows() - 1 << "): ";
            std::cin >> row;
            std::cout << "Enter column (0-" << sparseMatrix->GetColumns() - 1 << "): ";
            std::cin >> col;

            try {
                double value = sparseMatrix->GetElement(row, col);
                std::cout << "[INFO] Value at (" << row << ", " << col << "): " << value << "\n";
            } catch (const std::exception &e) {
                std::cerr << "Error: " << e.what() << "\n";
            }

        } else if (choice == 3) {
            updateMatrixDisplay(*sparseMatrix);
        } else if (choice == 4) {
            break;
        } else {
            std::cout << "Invalid choice. Try again.\n";
        }
    }
}

// Обновление отображения SparseVector
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

// Обновление отображения SparseMatrix
void updateMatrixDisplay(const SparseMatrix<double> &sparseMatrix) {
    std::cout << "\nSparse Matrix Elements:\n";
    for (int i = 0; i < sparseMatrix.GetRows(); ++i) {
        for (int j = 0; j < sparseMatrix.GetColumns(); ++j) {
            try {
                double value = sparseMatrix.GetElement(i, j);
                std::cout << value << "\t";
            } catch (const std::exception &) {
                std::cout << "0\t";
            }
        }
        std::cout << "\n";
    }
}
