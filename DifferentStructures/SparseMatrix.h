#ifndef SPARSEMATRIX_H
#define SPARSEMATRIX_H

#include <vector>
#include <stdexcept>
#include <iostream>

template<typename TElement>
class SparseMatrix {
public:
    SparseMatrix(int rows, int columns)
            : rows(rows), columns(columns), elements(rows, std::vector<TElement>(columns, TElement())) {}

    int GetRows() const {
        return rows;
    }

    int GetColumns() const {
        return columns;
    }

    // Получение элемента матрицы
    TElement GetElement(int row, int column) const {
        CheckBounds(row, column);
        return elements[row][column];
    }

    // Установка элемента в матрицу
    void SetElement(int row, int column, const TElement& value) {
        CheckBounds(row, column);
        elements[row][column] = value;
    }

    // Удаление элемента (устанавливаем в значение по умолчанию)
    void RemoveElement(int row, int column) {
        CheckBounds(row, column);
        elements[row][column] = TElement();
    }

    // Применение функции ко всем элементам
    void ForEach(void (*func)(int, int, const TElement&)) const {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < columns; ++j) {
                if (elements[i][j] != TElement()) {
                    func(i, j, elements[i][j]);
                }
            }
        }
    }

    // Применение функции ко всем элементам
#include <functional>  // Добавляем для std::function

    void Map(std::function<TElement(TElement)> func) {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < columns; ++j) {
                elements[i][j] = func(elements[i][j]);
            }
        }
    }

    // Функция умножения всех элементов матрицы на число
    void MultiplyByScalar(TElement scalar) {
        if (scalar == 0) {
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < columns; ++j) {
                    elements[i][j] = TElement();
                }
            }
        } else {
            Map([scalar](TElement x) { return x * scalar; });
        }
    }

    TElement Reduce(TElement (*func)(TElement, TElement), TElement initial) const {
        TElement result = initial;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < columns; ++j) {
                result = func(result, elements[i][j]);
            }
        }
        return result;
    }

private:
    int rows;
    int columns;
    std::vector<std::vector<TElement>> elements;

    void CheckBounds(int row, int column) const {
        if (row < 0 || row >= rows || column < 0 || column >= columns) {
            throw std::out_of_range("Row or column index is out of bounds.");
        }
    }
};

#endif // SPARSEMATRIX_H
