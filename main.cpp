#include <QApplication>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include "test_sparse_vector.h"
#include "test_sparse_matrix.h"
#include "test.h"
#include "menu.h"
#include "DifferentStructures/BTree.h"
#include "test_hashtable.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <utility>
#include <QVBoxLayout>
#include <chrono>
#include "test_btree.h"

std::vector<std::vector<int>> loadMatrixFromFile(const std::string& filename) {
    std::ifstream infile(filename);

    // Проверяем, удалось ли открыть файл
    if (!infile.is_open()) {
        std::cerr << "Error: Unable to open file: " << filename << std::endl;
        throw std::runtime_error("Failed to open file: " + filename);
    }

    int rows = 0, cols = 0;
    std::string line;

    // Считываем количество строк и столбцов
    if (!(infile >> line >> rows)) { // "rows <число>"
        throw std::runtime_error("Error reading rows from file");
    }
    if (!(infile >> line >> cols)) { // "cols <число>"
        throw std::runtime_error("Error reading cols from file");
    }

    // Начало измерения времени
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::vector<int>> matrix(rows, std::vector<int>(cols));

    // Считываем данные построчно
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (!(infile >> matrix[i][j])) {
                throw std::runtime_error("Invalid matrix data in file");
            }
        }
    }

    infile.close();

    // Конец измерения времени
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "File loaded successfully: " << filename << std::endl;
    std::cout << "Matrix creation time: " << elapsed.count() << " seconds." << std::endl;

    return matrix;
}

void printMatrix(const std::vector<std::vector<int>>& matrix) {
    for (const auto& row : matrix) {
        for (int value : row) {
            std::cout << value << " ";
        }
        std::cout << "\n";
    }
}

void save_data_to_file(const std::string& filename, const std::vector<Point>& data) {
    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    for (const auto& point : data) {
        outfile << point.first << " " << point.second << "\n";
    }
    outfile.close();
}

void plotGraphQt(const std::vector<std::pair<double, double>>& data, const QString& title) {
    QLineSeries* series = new QLineSeries();

    for (const auto& point : data) {
        series->append(point.second, point.first); // X = Number of Elements, Y = Time
    }

    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(title);
    chart->createDefaultAxes();

    QValueAxis* axisX = new QValueAxis();
    axisX->setTitleText("Number of Elements");
    axisX->setLabelFormat("%d");
    chart->setAxisX(axisX, series);

    QValueAxis* axisY = new QValueAxis();
    axisY->setTitleText("Time");
    axisY->setLabelFormat("%.2f");
    chart->setAxisY(axisY, series);

    QChartView* chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QWidget* window = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(window);
    layout->addWidget(chartView);

    window->setWindowTitle(title);
    window->resize(800, 600);
    window->show();
}

void TestBTree() {
    BTree<int, std::string> tree1;
    tree1.Add(10, "Ten");
    tree1.PrintStructure();
    tree1.Add(20, "Twenty");
    tree1.PrintStructure();
    tree1.Add(5, "Five");
    tree1.PrintStructure();
    tree1.Add(15, "Fifteen");
    tree1.PrintStructure();
    BTree<int, std::string> tree;

    tree.Add(10, "Ten");
    tree.Add(20, "Twenty");
    tree.Add(5, "Five");
    tree.Add(6, "Six");
    tree.Add(12, "Twelve");
    tree.Add(30, "Thirty");
    tree.Add(7, "Seven");
    tree.Add(17, "Seventeen");

    std::cout << "B-Tree structure after additions:\n";
    tree.PrintStructure();

    std::cout << "\nSearch tests:\n";
    std::cout << "Search 6: " << (tree.ContainsKey(6) ? "Found" : "Not Found") << "\n";
    std::cout << "Search 15: " << (tree.ContainsKey(15) ? "Found" : "Not Found") << "\n";

    tree.Remove(6);
    std::cout << "\nB-Tree structure after removing 6:\n";
    tree.PrintStructure();
    try {
        tree.Remove(13);
        std::cout << "\nB-Tree structure after removing 13 (non-existent):\n";
    } catch (const std::exception& e) {
        std::cout << "EXPECTED EXCEPTION\n";
        std::cout << e.what() << "\n";
    }
    tree.PrintStructure();
}

void Menu(int argc, char* argv[]) {
    int choice;
    do {
        std::cout << "\n===== MAIN MENU =====\n";
        std::cout << "1. Test BTree\n";
        std::cout << "2. Test Sparse Matrix\n";
        std::cout << "3. Test Sparse Vector\n";
        std::cout << "4. Generate Performance Graphs\n";
        std::cout << "5. Manual Trying\n";
        std::cout << "6. Plot Graph from File\n";
        std::cout << "7. Run All Tests\n";
        std::cout << "8. Test Hashtable\n";
        std::cout << "9. Test General BTree\n";\
        std::cout << "0. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice) {
            case 1:
                TestBTree();
                break;
            case 2:
                test_sparse_matrix();
                break;
            case 3:
                test_sparse_vector();
                break;
            case 4:
                try {
                    QApplication app(argc, argv);

                    // Generate data for graphs
                    const std::vector<Point> result_time_matrix_hash = {
                            {load_test_sparse_matrix(10000000, 100000).first, 100000},
                            {load_test_sparse_matrix(5000000, 50000).first, 50000},
                            {load_test_sparse_matrix(1000000, 10000).first, 10000},
                            {load_test_sparse_matrix(500000, 5000).first, 5000},
                            {load_test_sparse_matrix(100000, 1000).first, 1000},
                            {load_test_sparse_matrix(50000, 500).first, 500},
                            {load_test_sparse_matrix(10000, 100).first, 100}
                    };

                    const std::vector<Point> result_time_matrix_b = {
                            {load_test_sparse_matrix(10000000, 100000).second, 100000},
                            {load_test_sparse_matrix(5000000, 50000).second, 50000},
                            {load_test_sparse_matrix(1000000, 10000).second, 10000},
                            {load_test_sparse_matrix(500000, 5000).second, 5000},
                            {load_test_sparse_matrix(100000, 1000).second, 1000},
                            {load_test_sparse_matrix(50000, 500).second, 500},
                            {load_test_sparse_matrix(10000, 100).second, 100}
                    };

                    const std::vector<Point> result_time_vector_hash = {
                            {load_test_sparse_vector(1000000000, 100000).first, 100000},
                            {load_test_sparse_vector(500000000, 50000).first, 50000},
                            {load_test_sparse_vector(100000000, 10000).first, 10000},
                            {load_test_sparse_vector(50000000, 5000).first, 5000},
                            {load_test_sparse_vector(10000000, 1000).first, 1000},
                            {load_test_sparse_vector(5000000, 500).first, 500},
                            {load_test_sparse_vector(1000000, 100).first, 100}
                    };

                    const std::vector<Point> result_time_vector_b = {
                            {load_test_sparse_vector(1000000000, 100000).second, 100000},
                            {load_test_sparse_vector(500000000, 50000).second, 50000},
                            {load_test_sparse_vector(100000000, 10000).second, 10000},
                            {load_test_sparse_vector(50000000, 5000).second, 5000},
                            {load_test_sparse_vector(10000000, 1000).second, 1000},
                            {load_test_sparse_vector(5000000, 500).second, 500},
                            {load_test_sparse_vector(1000000, 100).second, 100}
                    };

                    // Display graphs using Qt
                    plotGraphQt(result_time_matrix_hash, "Matrix Hash Performance");
                    plotGraphQt(result_time_matrix_b, "Matrix BTree Performance");
                    plotGraphQt(result_time_vector_hash, "Vector Hash Performance");
                    plotGraphQt(result_time_vector_b, "Vector BTree Performance");

                    // Run the application
                    app.exec();
                } catch (const std::exception& e) {
                    std::cerr << "Error: " << e.what() << std::endl;
                }
                break;
            case 5:
            {
                displayMenu();
                };
                break;
            case 6: {
                try {
                    std::string filename;
                    std::cout << "Enter filename for matrix data: ";
                    std::cin >> filename;

                    auto matrix = loadMatrixFromFile(filename);

                    std::cout << "Matrix loaded successfully:\n";
                    printMatrix(matrix);
                } catch (const std::exception& e) {
                    std::cerr << "Error: " << e.what() << "\n";
                }
                break;
            }
            case 7:
                run_tests();
                break;
            case 8:
                test_hashtable();
                break;
            case 9:{
                std::cout << "Starting BTree tests..." << std::endl;

                // Создаем экземпляр тестового класса с порядком дерева (например, 3)
                BTreeTest btreeTest(3);

                // Добавление элементов
                btreeTest.AddElement(10, "Value10");
                btreeTest.AddElement(20, "Value20");
                btreeTest.AddElement(5, "Value5");
                btreeTest.AddElement(15, "Value15");
                btreeTest.AddElement(25, "Value25");

                // Удаление элемента
                btreeTest.RemoveElement(20);

                // Отображение дерева
                std::cout << "\nDisplaying tree:" << std::endl;
                btreeTest.DisplayTree();

                // Проверка баланса дерева
                std::cout << "\nChecking tree balance:" << std::endl;
                btreeTest.CheckBalance();

                // Заполнение случайными значениями
                std::cout << "\nFilling tree with random elements:" << std::endl;
                btreeTest.FillRandom(10, 1, 50);

                // Отображение дерева после заполнения
                std::cout << "\nDisplaying tree after random fill:" << std::endl;
                btreeTest.DisplayTree();

                std::cout << "\nAll tests completed!" << std::endl;}
                break;
            case 0:
                std::cout << "Exiting...\n";
                break;
            default:
                std::cout << "Invalid choice. Please try again.\n";
                break;
        }
    } while (choice != 0);
}

int main(int argc, char* argv[]) {
    Menu(argc, argv);
    return 0;
}
