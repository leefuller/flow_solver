#ifndef INCLUDE_MATRIX_H
#define INCLUDE_MATRIX_H

#include <tuple>
#include <stdexcept>

template<typename T> class Matrix
{
  public:
	/**
	 * Create matrix
     * @param rowCould      Number of rows
     * @param colCount      Number of columns
	 */
    Matrix (unsigned rowCount, unsigned colCount)
      : m_rowCount(rowCount), m_colCount(colCount)
    {
        m_values = new T*[rowCount];
        for (unsigned i = 0; i < rowCount; ++i)
            m_values[i] = new T[colCount];
    }

    ~Matrix ()
    {
        for (unsigned i = 0; i < m_rowCount; ++i)
            delete[] m_values[i];
        delete[] m_values;
    }

	/**
	 * @return number of rows in matrix
	 */
    unsigned getRowCount () const noexcept
    { return m_rowCount; }

	/**
	 * @return number of columns in matrix
	 */
    unsigned getColumnCount () const noexcept
    { return m_colCount; }

	/**
     * Get item at given location
	 * @param row	Row index
	 * @param col	Columnm index
	 * @return const reference to item at position given by row and col
	 */
    const T & at (int row, int col) const noexcept(false)
    {
        if (row >= getRowCount())
            throw std::range_error("row index out of range");
        if (col >= getColumnCount())
            throw std::range_error("column index out of range");
        return m_values[row][col];
    }

	/**
     * Get item at given location
	 * @param row	Row index
	 * @param col	Columnm index
	 * @return const reference to item at position given by row and col
	 */
    T & at (int row, int col) noexcept(false)
    {
        if (row >= getRowCount())
            throw std::range_error("row index out of range");
        if (col >= getColumnCount())
            throw std::range_error("column index out of range");
        return m_values[row][col];
    }

    /**
     * Get item at given location
     * @return const reference to item at position given by row and col
     */
    T & at (const std::array<int, 2> & pos) noexcept(false)
    {
        return at(pos[0], pos[1]);
    }

	/**
	 * Set all matrix points to the same value.
	 */
    void setAllValues (T & val)
    {
        for (unsigned r = 0; r < getRowCount(); ++r)
            for (unsigned c = 0; c < getColumnCount(); ++c)
                at(r, c) = val;
    }

	/**
     * Set all matrix points to the same value.
	 */
    void setAllValues (T && val)
    {
		return setAllValues(val);
    }

  private:
    unsigned m_rowCount;
    unsigned m_colCount;

    T ** m_values;
};
#endif
