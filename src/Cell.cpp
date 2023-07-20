#include "Cell.h"
#include "PuzzleRepr.h"
#include "PuzzleException.h"

#include <sstream>
#include <iostream>

/**
 * Factory function to create a new Cell object
 */
CellPtr Cell::createCell (Coordinate c, PipeId idPipe) noexcept
{
    //CellPtr p = std::make_shared<Cell>();
    CellPtr p = std::shared_ptr<Cell>(new Cell);
    p->setCoordinate(c);
    return p;
}

bool Cell::operator== (const Cell & c) const noexcept
{
    if (getPipeId() != c.getPipeId())
        return false;
    if (getCoordinate() != c.getCoordinate())
        return false;
    if (getBorder(Direction::NORTH) != c.getBorder(Direction::NORTH))
        return false;
    if (getConnection(Direction::NORTH) != c.getConnection(Direction::NORTH))
        return false;
    if (getBorder(Direction::SOUTH) != c.getBorder(Direction::SOUTH))
        return false;
    if (getConnection(Direction::SOUTH) != c.getConnection(Direction::SOUTH))
        return false;
    if (getBorder(Direction::WEST) != c.getBorder(Direction::WEST))
        return false;
    if (getConnection(Direction::WEST) != c.getConnection(Direction::WEST))
        return false;
    if (getBorder(Direction::EAST) != c.getBorder(Direction::EAST))
        return false;
    if (getConnection(Direction::EAST) != c.getConnection(Direction::EAST))
        return false;
    // TODO possiblePipes
    if (getEndpoint() != c.getEndpoint())
        return false;
    return true;
}

bool Cell::outputConnectorRep = false;

void Cell::setOutputConnectorRep (bool repr) noexcept
{ outputConnectorRep = repr; }

extern std::ostream & outputBorderRepr (std::ostream & os, Direction direction, CellBorder border);
extern std::ostream & outputConnectionRepr (std::ostream & os, Direction direction, CellConnection connection);

std::ostream & operator<< (std::ostream & os, const Cell & cell) noexcept
{
    os << (!cell.isBorderOpen(Direction::WEST) ? VERTICAL_WALL_DEF_CH : ' ');
    if (Cell::isOutputConnectorRep())
        outputConnectionRepr(os, Direction::WEST, cell.getConnection(Direction::WEST));
    os << (cell.isEmpty() ? EMPTY_CELL_DEF_CH : cell.getPipeId());
    if (Cell::isOutputConnectorRep())
        outputConnectionRepr(os, Direction::EAST, cell.getConnection(Direction::EAST));
    os << (!cell.isBorderOpen(Direction::EAST) ? VERTICAL_WALL_DEF_CH : ' ');
    return os;
}

std::ostream & operator<< (std::ostream & os, const CellPtr cell) noexcept
{ return os << *cell; }

std::ostream & operator<< (std::ostream & os, const std::vector<CellPtr> & row) noexcept
//std::ostream & operator<< (std::ostream & os, const PuzzleRow & row) noexcept
{
#if 0
    // Upper border (typically just complicates the look by showing horizontal borders between cells twice.)
    // If left out, the slightly negative impact is that top border of the puzzle is not output.
    for (ConstCellPtr cell : row)
    {
        os << (cell->getBorder(Direction::WEST) == CellBorder::WALL ? VERTICAL_WALL_DEF_CH : ' ');
        outputBorderRepr(os, Direction::NORTH, cell->getBorder(Direction::NORTH));
        os << (cell->getBorder(Direction::EAST) == CellBorder::WALL ? VERTICAL_WALL_DEF_CH : ' ');
    }
    os << std::endl;
#endif

    if (Cell::isOutputConnectorRep())
    {
        // Upper connector
        for (ConstCellPtr cell : row)
        {
            os << (!cell->isBorderOpen(Direction::WEST) ? VERTICAL_WALL_DEF_CH : ' ');
            outputConnectionRepr(os, Direction::NORTH, cell->getConnection(Direction::NORTH));
            os << (!cell->isBorderOpen(Direction::EAST) ? VERTICAL_WALL_DEF_CH : ' ');
        }
        os << std::endl;
    }

    for (ConstCellPtr cell : row)
        os << *cell;
    os << std::endl;

    if (Cell::isOutputConnectorRep())
    {
        // Lower connector
        for (ConstCellPtr cell : row)
        {
            os << (!cell->isBorderOpen(Direction::WEST) ? VERTICAL_WALL_DEF_CH : ' ');
            outputConnectionRepr(os, Direction::SOUTH, cell->getConnection(Direction::SOUTH));
            os << (!cell->isBorderOpen(Direction::EAST) ? VERTICAL_WALL_DEF_CH : ' ');
        }
        os << std::endl;
    }

#if 1
    // Lower border
    for (ConstCellPtr cell : row)
    {
        os << (cell->getBorder(Direction::WEST) == CellBorder::WALL ? VERTICAL_WALL_DEF_CH : ' ');
        outputBorderRepr(os, Direction::SOUTH, cell->getBorder(Direction::SOUTH));
        os << (cell->getBorder(Direction::EAST) == CellBorder::WALL ? VERTICAL_WALL_DEF_CH : ' ');
    }
    os << std::endl;
#endif
return os;
}

std::string Cell::toString () const noexcept
{
    std::ostringstream ss;
    ss << (getBorder(Direction::WEST) == CellBorder::WALL ? VERTICAL_WALL_DEF_CH : ' ');
    ss << (getPipeId() == NO_PIPE_ID ? '.' : (char)getPipeId());
    ss << (getBorder(Direction::EAST) == CellBorder::WALL ? VERTICAL_WALL_DEF_CH : ' ');
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
        throw PuzzleException(SOURCE_REF, "Cell attempt to change fixed connection at [%u,%u]", std::get<0>(getCoordinate()), std::get<1>(getCoordinate()));
    if (m_connection[Direction::NORTH] == from)
        setConnection(Direction::NORTH, to);
    if (m_connection[Direction::SOUTH] == from)
        setConnection(Direction::SOUTH, to);
    if (m_connection[Direction::WEST] == from)
        setConnection(Direction::WEST, to);
    if (m_connection[Direction::EAST] == from)
        setConnection(Direction::EAST, to);
    // Diagonal have no connections to centre
}

/**
 * Determine whether a new connection can be made in the given direction.
 * @return true if a new connection can be made in the given direction
 */
bool Cell::canAcceptConnection (Direction d) const noexcept
{
    if (isDiagonal(d))
        return false;
    if (!isBorderOpen(d))
        return false;
    if (getConnection(d) == CellConnection::NO_CONNECTOR)
        return false;
    if (isEndpoint() && countFixtureConnections() == 1)
        return false;
    return countFixtureConnections() < 2 && getConnection(d) != CellConnection::FIXTURE_CONNECTION;
}
