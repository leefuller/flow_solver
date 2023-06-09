#include "../app/Matrix.h"
#include <gtest/gtest.h>

#include <exception>
#include <iostream>

TEST(matrix_test, set_all_values)
{
    unsigned numRows = 5;
    unsigned numCols = 3;
    Matrix<int> m(numRows, numCols);
    EXPECT_TRUE(m.getRowCount() == numRows);
    EXPECT_TRUE(m.getColumnCount() == numCols);
    m.setAllValues(-1);
    for (unsigned r = 0; r < m.getRowCount(); ++r)
    {
        for (unsigned c = 0; c < m.getColumnCount(); ++c)
        {
            EXPECT_TRUE(m.at(r,c) == -1);
        }
    }
}

TEST(matrix_test, set_a_value)
{
    unsigned numRows = 5;
    unsigned numCols = 3;
    Matrix<int> m(numRows, numCols);
    m.setAllValues(-1);
    m.at(1, 2) = 5;
    EXPECT_TRUE(m.at(1, 2) == 5);
    EXPECT_TRUE(m.at(std::array<int,2>({1, 2})) == 5);

    EXPECT_THROW(m.at(5, 0), std::range_error);
    EXPECT_THROW(m.at(0, 3), std::range_error);
}
