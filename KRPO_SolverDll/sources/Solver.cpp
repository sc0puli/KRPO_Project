#include "Solver.hpp"
#include <Windows.h>
#include <cmath>
#include <iostream>

void Solver_Gauss::Solve(double **Y, double *x, double *I, int sleSize) {
    // ������ ��� ������ ������
    for (int i = 0; i < sleSize; i++) {
        // ����� �������� �������� ��� ������� ������
        int maxRow = i;
        for (int k = i + 1; k < sleSize; k++) {
            if (fabs(Y[k][i]) > fabs(Y[maxRow][i])) {
                maxRow = k;
            }
        }

        // ������������ �����
        for (int k = i; k < sleSize; k++) {
            std::swap(Y[maxRow][k], Y[i][k]);
        }
        std::swap(I[maxRow], I[i]);

        // �������� �� ������������� �������
        if (fabs(Y[i][i]) < 1e-9) {
            std::cerr << "������� ���������, ������� ����������." << std::endl;
            return;
        }

        // ������ ��� (���������� � ������������ ����)
        for (int k = i + 1; k < sleSize; k++) {
            double factor = Y[k][i] / Y[i][i];
            for (int j = i; j < sleSize; j++) {
                Y[k][j] -= factor * Y[i][j];
            }
            I[k] -= factor * I[i];
        }
    }

    // �������� ��� ������ ������
    for (int i = sleSize - 1; i >= 0; i--) {
        x[i] = I[i];
        for (int j = i + 1; j < sleSize; j++) {
            x[i] -= Y[i][j] * x[j];
        }
        x[i] /= Y[i][i];
    }
}

PluginType GetType() {
    return PluginType::solver;
}
void GetStringID(std::string &_id) {
    _id = "Gauss";
}

Solver *GetSolver() {
    return new Solver_Gauss;
}

void FreeSolver(Solver *_p_solver) {
    if (_p_solver) {
        delete _p_solver;
    _p_solver = nullptr;
    }
}