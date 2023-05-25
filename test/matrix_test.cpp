#include "../app/Matrix.h"
#include "test_helper.h"

#include <exception>
#include <iostream>

int main ()
{
    bool result = true;
    unsigned numRows = 5;
    unsigned numCols = 3;
    Matrix<int> m(numRows, numCols);
    result = result && check(m.getRowCount() == numRows);
    result = result && check(m.getColumnCount() == numCols);
    m.setAllValues(-1);
    for (unsigned r = 0; r < m.getRowCount(); ++r)
    {
        for (unsigned c = 0; c < m.getColumnCount(); ++c)
        {
            result = result && check(m.at(r,c) == -1);
        }
    }
    m.at(1, 2) = 5;
    result = result && check(m.at(1, 2) == 5);
    result = result && check(m.at(std::array<unsigned,2>({1, 2})) == 5);
    try { m.at(5, 0); }
    catch (const std::range_error & ex) {} // expected
    catch (...) { throw "unexpected throw"; }
    try { m.at(0, 3); }
    catch (const std::range_error & ex) {} // expected
    catch (...) { throw "unexpected throw"; }

    // TODO matrix of complex type for:
    //m.setAllValues (T && val);
    return result ? 0 : 1;
}
