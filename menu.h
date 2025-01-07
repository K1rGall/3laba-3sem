#pragma once

#include "DifferentStructures/SparseVector.h"
#include "DifferentStructures/SparseMatrix.h"
#include "DifferentStructures/UnqPtr.h"
using Point = std::pair<double, double>;

void handleVectorOperations(UnqPtr<SparseVector<double>> &sparseVector);
void handleMatrixOperations(UnqPtr<SparseMatrix<double>> &sparseMatrix);
void updateVectorDisplay(const SparseVector<double> &sparseVector);
void updateMatrixDisplay(const SparseMatrix<double> &sparseMatrix);

void displayMenu();


