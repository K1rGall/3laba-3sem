#include "test.h"
#include "DifferentStructures/SparseVector.h"
#include "DifferentStructures/SparseMatrix.h"
#include "DifferentStructures/BTree.h"
#include "DifferentStructures/UnqPtr.h"
#include "DifferentStructures/HashTable.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <string>
#include <cstdlib>
#include <unordered_set>
#include <algorithm>
#include <random>

void run_tests() {
    std::cout << "Executing functional checks..." << std::endl;
    functional_tests();
    std::cout << "Functional tests done." << std::endl;

    std::cout << "\nRunning performance evaluations..." << std::endl;
    performance_tests();
    std::cout << "Performance evaluations completed." << std::endl;
}

void functional_tests() {
    test_dictionary<HashTable<int, std::string>, int, std::string>("HashTable");
    test_dictionary<BTree<int, std::string>, int, std::string>("BTree");

    test_sparse_vector<HashTable<int, double>>("HashTable", true);
    test_sparse_vector<BTree<int, double>>("BTree", true);

    test_sparse_matrix<HashTable<IndexPair, double>>("HashTable", true);
    test_sparse_matrix<BTree<IndexPair, double>>("BTree", true);

    std::cout << "All functional verifications succeeded." << std::endl;
}

template <typename DictionaryType, typename KeyType, typename ValueType>
void test_dictionary(const std::string& dictionary_name) {
    std::cout << "Evaluating " << dictionary_name << "..." << std::endl;
    DictionaryType dictionary;

    try {
        dictionary.Add(101, "Alpha");
        dictionary.Add(202, "Beta");
        dictionary.Add(303, "Gamma");

        ValueType value = dictionary.Get(202);
        if (value != "Beta") {
            std::cerr << "Error: expected 'Beta', got '" << value << "'" << std::endl;
        } else {
            std::cout << "Get(202) successful, value: " << value << std::endl;
        }

        dictionary[202] = "BetaUpdated";
        ValueType updatedValue = dictionary.Get(202);
        if (updatedValue != "BetaUpdated") {
            std::cerr << "Error: expected 'BetaUpdated', got '" << updatedValue << "'" << std::endl;
        } else {
            std::cout << "Update successful, new value of Get(202): " << updatedValue << std::endl;
        }

        dictionary.Remove(303);
        if (dictionary.ContainsKey(303)) {
            std::cerr << "Error: Key 303 should have been removed." << std::endl;
        } else {
            std::cout << "Remove successful, key 303 no longer exists in the " << dictionary_name << "." << std::endl;
        }

        std::cout << "Iterating over " << dictionary_name << ":" << std::endl;
        auto iterator = dictionary.GetIterator();
        while (iterator->MoveNext()) {
            std::cout << "Key: " << iterator->GetCurrentKey()
                      << ", Value: " << iterator->GetCurrentValue() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception in " << dictionary_name << " tests: " << e.what() << std::endl;
    }
}


template <typename DictionaryType>
void test_sparse_vector(const std::string& dictionary_name, bool extended) {
    std::cout << "Testing SparseVector with " << dictionary_name << "..." << std::endl;
    UnqPtr<IDictionary<int, double>> dictionary(new DictionaryType());
    SparseVector<double> vector(10, std::move(dictionary));

    try {
        vector.SetElement(2, 3.5);
        double value = vector.GetElement(2);
        std::cout << "GetElement(2) succeeded, value: " << value << std::endl;

        vector.Map([](double x) { return x * 2; });
        double mappedValue = vector.GetElement(2);
        std::cout << "Map succeeded, new value of GetElement(2): " << mappedValue << std::endl;

        vector.RemoveElement(2);
        if (vector.GetElement(2) != 0.0) {
            std::cerr << "Error: Element at index 2 should have been removed." << std::endl;
        } else {
            std::cout << "RemoveElement succeeded, element at index 2 is now zero." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception in SparseVector tests: " << e.what() << std::endl;
    }
}

template <typename DictionaryType>
void test_sparse_matrix(const std::string& dictionary_name, bool extended) {
    std::cout << "Testing SparseMatrix with " << dictionary_name << "..." << std::endl;
    UnqPtr<IDictionary<IndexPair, double>> dictionary(new DictionaryType());
    SparseMatrix<double> matrix(4, 4);

    // Добавляем элементы в матрицу
    matrix.SetElement(0, 0, 1.0);
    matrix.SetElement(1, 1, 2.0);
    matrix.SetElement(2, 2, 3.0);

    // Проверим состояние матрицы перед вызовом Map()
    std::cout << "SparseMatrix before Map:" << std::endl;
    matrix.ForEach([](int row, int column, const double& value) {
        std::cout << "Position: (" << row << ", " << column << "), Value: " << value << std::endl;
    });

    matrix.Map([](double x) { return x + 1; });

    // Проверим состояние матрицы после Map()
    std::cout << "SparseMatrix after Map:" << std::endl;
    matrix.ForEach([](int row, int column, const double& value) {
        std::cout << "Position: (" << row << ", " << column << "), Value: " << value << std::endl;
    });

    // Вычисляем сумму элементов через Reduce
    double sum = matrix.Reduce([](double acc, double x) { return acc + x; }, 0.0);
    std::cout << "Calculated Reduce sum: " << sum << std::endl;
}

template<typename Func>
long long measure_time(Func func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto finish = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
}

template<typename TDictionary>
void performance_test_vector(int size, const std::string& dict_name, std::ostream& log_stream) {
    UnqPtr<IDictionary<int, double>> dictionary(new TDictionary());
    SparseVector<double> vector(size, std::move(dictionary));

    std::unordered_set<int> indices;
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dis(0, size - 1);

    while (indices.size() < static_cast<size_t>(size / 10)) {
        indices.insert(dis(gen));
    }

    long long insertion_time = measure_time([&]() {
        for (int idx : indices) {
            vector.SetElement(idx, static_cast<double>(std::rand()) / RAND_MAX);
        }
    });

    long long search_time = measure_time([&]() {
        for (int idx : indices) {
            vector.GetElement(idx);
        }
    });

    log_stream << dict_name << ",Vector," << size << "," << indices.size() << ","
               << insertion_time << "," << search_time << "\n";
}

template<typename TDictionary>
void performance_test_matrix(int size, const std::string& dict_name, std::ostream& log_stream) {
    int rows = std::max(1, size);
    int cols = std::max(1, size);
    UnqPtr<IDictionary<IndexPair, double>> dictionary(new TDictionary());
    SparseMatrix<double> matrix(rows, cols);

    long long total_elements = (long long)rows * (long long)cols;
    long long num_elements = std::max(1LL, total_elements / 10LL);
    std::unordered_set<long long> index_set;
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dis_row(0, rows - 1);
    std::uniform_int_distribution<> dis_col(0, cols - 1);

    long long insertion_time = measure_time([&]() {
        while (index_set.size() < (size_t)num_elements) {
            int i = dis_row(gen);
            int j = dis_col(gen);
            index_set.insert((long long)i * (long long)cols + j);
        }

        for (long long key : index_set) {
            int i = (int)(key / cols);
            int j = (int)(key % cols);
            matrix.SetElement(i, j, static_cast<double>(std::rand()) / RAND_MAX);
        }
    });

    std::vector<IndexPair> indices;
    indices.reserve(index_set.size());
    for (long long key : index_set) {
        int i = (int)(key / cols);
        int j = (int)(key % cols);
        indices.emplace_back(i, j);
    }

    long long search_time = measure_time([&]() {
        for (const auto& idx : indices) {
            matrix.GetElement(idx.row, idx.column);
        }
    });

    long long map_time = measure_time([&]() {
        matrix.Map([](double x) { return x + 1; });
    });

    long long reduce_time = measure_time([&]() {
        matrix.Reduce([](double acc, double x) { return acc + x; }, 0.0);
    });

    long long update_time = measure_time([&]() {
        for (const auto& idx : indices) {
            matrix.SetElement(idx.row, idx.column, static_cast<double>(std::rand()) / RAND_MAX);
        }
    });

    long long iteration_time = measure_time([&]() {
        for (int i = 0; i < matrix.GetRows(); ++i) {
            for (int j = 0; j < matrix.GetColumns(); ++j) {
                volatile double val = matrix.GetElement(i, j);
                (void)val;
            }
        }
    });

    log_stream << dict_name << ",Matrix," << size << "," << num_elements << ","
               << insertion_time << "," << search_time << "," << map_time << ","
               << reduce_time << "," << update_time << "," << iteration_time << "\n";
}

std::vector<int> read_test_sizes(const std::string& filename) {
    std::vector<int> sizes;
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Failed to open configuration file: " << filename << std::endl;
        return sizes;
    }

    int size;
    while (file >> size) {
        sizes.push_back(size);
    }
    return sizes;
}

void performance_tests() {
    std::vector<int> sizes = read_test_sizes("config.txt");
    if (sizes.empty()) {
        std::cerr << "Cannot read sizes from config.txt" << std::endl;
        return;
    }

    std::ofstream log_file("performance_results.csv");
    if (!log_file.is_open()) {
        std::cerr << "Cannot open the file performance_results.csv for writing." << std::endl;
        return;
    }

    log_file << "Dictionary,Structure,Size,NumElements,InsertionTime(ms),SearchTime(ms),MapTime(ms),ReduceTime(ms),UpdateTime(ms),IterationTime(ms)\n";

    for (size_t i = 0; i < sizes.size(); ++i) {
        int size = sizes[i];
        std::cout << "\nTesting with data size: " << size << std::endl;

        std::cout << "Starting performance test for size: " << size << std::endl;

        if (i % 2 == 0) {
            std::cout << "Running vector tests for size: " << size << std::endl;
            performance_test_vector<HashTable<int, double>>(size, "HashTable", log_file);
            std::cout << "Completed HashTable vector test for size: " << size << std::endl;

            performance_test_vector<BTree<int, double>>(size, "BTree", log_file);
            std::cout << "Completed BTree vector test for size: " << size << std::endl;
        } else {
            std::cout << "Running matrix tests for size: " << size << std::endl;
            performance_test_matrix<HashTable<IndexPair, double>>(size, "HashTable", log_file);
            std::cout << "Completed HashTable matrix test for size: " << size << std::endl;

            performance_test_matrix<BTree<IndexPair, double>>(size, "BTree", log_file);
            std::cout << "Completed BTree matrix test for size: " << size << std::endl;
        }
    }

    log_file.close();
    std::cout << "Performance tests completed. Results saved in performance_results.csv" << std::endl;
}
