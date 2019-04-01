#include<iostream>

class Row {
    int *row;
    const size_t len;

    public:
        Row(int *row, size_t len) : row(row), len(len) {}

        int operator[] (size_t num) const {
            if (num >= len) 
                throw std::out_of_range("");
            return row[num];
        }

        int &operator[] (size_t num) {
            if (num >= len) 
                throw std::out_of_range("");
            return row[num];
        }
};

class Matrix {
    const size_t rows;
    const size_t cols;
    int **matrix;

    public:
        Matrix(size_t rows1, size_t cols1) :rows(rows1), cols(cols1) {
            matrix = new int *[rows];
            for (int i = 0; i < rows; ++i) {
                matrix[i] = new int [cols];
            }
        }

        ~Matrix() {
            for (int i = 0; i < rows; ++i) {
                delete[] matrix[i];
            }
            delete[] matrix;
        }
        
        size_t getRows() const {
            return rows;
        }
        
        size_t getColumns() const {
            return cols;
        }

        const Row operator[](size_t num) const {
            if (num >= rows) 
                throw std::out_of_range("");
            return Row(matrix[num], cols);
        }

        Row operator[](size_t num) {
            if (num >= rows) 
                throw std::out_of_range("");
            return Row(matrix[num], cols);
        }

        Matrix &operator*=(const int multiplier) {
            for (int i = 0; i < rows; ++i) 
               for (int j = 0; j < cols; ++j) 
                  matrix[i][j] *= multiplier;
            return *this;
        }

        bool operator==(const Matrix &second) const {
            if ((cols == second.cols) && (rows == second.rows)) {
                for (int i = 0; i < rows; ++i)
                    for (int j = 0; j < cols; ++j) 
                       if (matrix[i][j] != second.matrix[i][j]) {
                            return false;
                       }
                return true;
            }
            else return false;
        }

        bool operator!=(const Matrix &second) const {
            return !(this == &second);
        }
};
