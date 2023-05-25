#include "Cell.h"
#include "PuzzleRepr.h"
#include "PuzzleException.h"

#include <sstream>
#include <iostream>

/**
 * Factory function to create a new Cell object
 */
std::shared_ptr<Cell> Cell::createCell (Coordinate c, PipeId idPipe) noexcept
{
    //std::shared_ptr<Cell> p = std::make_shared<Cell>();
    std::shared_ptr<Cell> p = std::shared_ptr<Cell>(new Cell);
    p->setCoordinate(c);
    return p;
}

bool Cell::operator== (const Cell & c) const noexcept
{
    if (getPipeId() != c.getPipeId())
        return false;
    if (getCoordinate() != c.getCoordinate())
        return false;
    if (getBorder(Direction::UP) != c.getBorder(Direction::UP))
        return false;
    if (getConnection(Direction::UP) != c.getConnection(Direction::UP))
        return false;
    if (getBorder(Direction::DOWN) != c.getBorder(Direction::DOWN))
        return false;
    if (getConnection(Direction::DOWN) != c.getConnection(Direction::DOWN))
        return false;
    if (getBorder(Direction::LEFT) != c.getBorder(Direction::LEFT))
        return false;
    if (getConnection(Direction::LEFT) != c.getConnection(Direction::LEFT))
        return false;
    if (getBorder(Direction::RIGHT) != c.getBorder(Direction::RIGHT))
        return false;
    if (getConnection(Direction::RIGHT) != c.getConnection(Direction::RIGHT))
        return false;
    if (getEndpoint() != c.getEndpoint())
        return false;
    return true;
}

extern std::ostream & outputBorderRepr (std::ostream & os, Direction direction, CellBorder border);
extern std::ostream & outputConnectionRepr (std::ostream & os, Direction direction, CellConnection connection);

std::ostream & operator<< (std::ostream & os, const Cell & cell) noexcept
{
    os << (!cell.isBorderOpen(Direction::LEFT) ? VERTICAL_WALL_DEF_CH : ' ');
    outputConnectionRepr(os, Direction::LEFT, cell.getConnection(Direction::LEFT));
    os << (cell.isEmpty() ? EMPTY_CELL_DEF_CH : cell.getPipeId());
    outputConnectionRepr(os, Direction::RIGHT, cell.getConnection(Direction::RIGHT));
    os << (!cell.isBorderOpen(Direction::RIGHT) ? VERTICAL_WALL_DEF_CH : ' ');
    return os;
}

std::ostream & operator<< (std::ostream & os, const std::shared_ptr<Cell> cell) noexcept
{ return os << *cell; }

std::ostream & operator<< (std::ostream & os, const std::vector<std::shared_ptr<Cell>> & row) noexcept
//std::ostream & operator<< (std::ostream & os, const PuzzleRow & row) noexcept
{
#if 0
    // Upper border (typically just complicates the look by showing horizontal borders between cells twice.)
    // If left out, the slightly negative impact is that top border of the puzzle is not output.
    for (std::shared_ptr<const Cell> cell : row)
    {
        os << (cell->getBorder(Direction::LEFT) == CellBorder::WALL ? VERTICAL_WALL_DEF_CH : ' ');
        outputBorderRepr(os, Direction::UP, cell->getBorder(Direction::UP));
        os << (cell->getBorder(Direction::RIGHT) == CellBorder::WALL ? VERTICAL_WALL_DEF_CH : ' ');
    }
    os << std::endl;
#endif

    // Upper connector
    for (std::shared_ptr<const Cell> cell : row)
    {
        os << (!cell->isBorderOpen(Direction::LEFT) ? VERTICAL_WALL_DEF_CH : ' ');
        outputConnectionRepr(os, Direction::UP, cell->getConnection(Direction::UP));
        os << (!cell->isBorderOpen(Direction::RIGHT) ? VERTICAL_WALL_DEF_CH : ' ');
    }
    os << std::endl;

    for (std::shared_ptr<const Cell> cell : row)
        os << *cell;
    os << std::endl;

    // Lower connector
    for (std::shared_ptr<const Cell> cell : row)
    {
        os << (!cell->isBorderOpen(Direction::LEFT) ? VERTICAL_WALL_DEF_CH : ' ');
        outputConnectionRepr(os, Direction::DOWN, cell->getConnection(Direction::DOWN));
        os << (!cell->isBorderOpen(Direction::RIGHT) ? VERTICAL_WALL_DEF_CH : ' ');
    }
    os << std::endl;

#if 1
    // Lower border
    for (std::shared_ptr<const Cell> cell : row)
    {
        os << (cell->getBorder(Direction::LEFT) == CellBorder::WALL ? VERTICAL_WALL_DEF_CH : ' ');
        outputBorderRepr(os, Direction::DOWN, cell->getBorder(Direction::DOWN));
        os << (cell->getBorder(Direction::RIGHT) == CellBorder::WALL ? VERTICAL_WALL_DEF_CH : ' ');
    }
    os << std::endl;
#endif
return os;
}

std::string Cell::toString () const noexcept
{
    std::ostringstream ss;
    ss << (getBorder(Direction::LEFT) == CellBorder::WALL ? VERTICAL_WALL_DEF_CH : ' ');
    ss << (getPipeId() == NO_PIPE_ID ? '.' : (char)getPipeId());
    ss << (getBorder(Direction::RIGHT) == CellBorder::WALL ? VERTICAL_WALL_DEF_CH : ' ');
    return ss.str();
}

std::ostream & Cell::describe (std::ostream & os) const noexcept
{
    os << "Cell at " << getCoordinate() << " pipe=";
    if (getPipeId() == NO_PIPE_ID)
        os << "none";
    else
        os <<  getPipeId();
   os << ", end=" << getEndpoint();
   os << ", borders: [ ";
   for (Direction d : allTraversalDirections)
   {
       os << d << '=' << borderStr[getBorder(d)] << "; ";
   }
    os  << "], connections: [";
    for (Direction d : allTraversalDirections)
    {
        os << d << '=' << connectionStr[getConnection(d)] << "; ";
    }
    os << "]";
    return os;
}

/**
 * Change any connections of 1 type to another.
 * @param from  Conenctors of this type are changed
 * @param to    Connectors of from type are changed to this type
 * @throw exception if attempting to change fixed connection.
 */
void Cell::changeConnections (CellConnection from, CellConnection to) noexcept(false)
{
    if (from == CellConnection::FIXTURE_CONNECTION && to != CellConnection::FIXTURE_CONNECTION)
        throw PuzzleException("Cell attempt to change fixed connection at [%u,%u]", std::get<0>(getCoordinate()), std::get<1>(getCoordinate()));
    if (m_connection[Direction::UP] == from)
        setConnection(Direction::UP, to);
    if (m_connection[Direction::DOWN] == from)
        setConnection(Direction::DOWN, to);
    if (m_connection[Direction::LEFT] == from)
        setConnection(Direction::LEFT, to);
    if (m_connection[Direction::RIGHT] == from)
        setConnection(Direction::RIGHT, to);
}

/**
 * Determine whether a new connection can be made in the given direction.
 * @return true if a new connection can be made in the given direction
 */
bool Cell::canAcceptConnection (Direction d) const noexcept
{
    if (!isBorderOpen(d))
        return false;
    if (getConnection(d) == CellConnection::NO_CONNECTOR)
        return false;
    if (isEndpoint() && countFixtureConnections() == 1)
        return false;
    return countFixtureConnections() < 2 && getConnection(d) != CellConnection::FIXTURE_CONNECTION;
}
