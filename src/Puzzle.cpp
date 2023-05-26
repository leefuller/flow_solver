#include "Puzzle.h"
#include "PuzzleException.h"
#include "Cell.h"
#include "Logger.h"

#include <memory>
#include <charconv> // to_chars
#include <iostream>
#include <map>

static Logger & logger = Logger::getDefaultLogger();

Puzzle::Puzzle (const PuzzleDefinition & def)
    : m_def(def), m_puzzleRows(def.generateRows())//, m_plumber(this)
{}

/**
 * Perform a deep copy.
 * ie. The pointers to cells in the new puzzle do not point into cells in p
 */
Puzzle::Puzzle (const Puzzle & p) :
    m_def(p.m_def)//, m_plumber(p.m_plumber)
{
    for (std::vector<CellPtr> row : p.m_puzzleRows)
    {
        std::vector<CellPtr> rowCopy;
        for (CellPtr cell : row)
            rowCopy.push_back(std::make_shared<Cell>(*cell)); // make a copy of the cell
        m_puzzleRows.push_back(rowCopy);
    }
}

/**
 * Output to stream in matrix format.
 */
std::ostream & Puzzle::streamPuzzleMatrix (std::ostream & os) const noexcept
{
    for (const PuzzleRow & row : m_puzzleRows)
    {
        os << row;
        //os << std::endl;
    }
    return os;
}

/**
 * Get the cell at the given coordinate.
 * @return pointer to cell, if existing
 */
CellPtr Puzzle::getCellAtCoordinate(Coordinate c) noexcept
{
    return std::const_pointer_cast<Cell>(getConstCellAtCoordinate(c));
}

/**
 * Get the cell at the given coordinate.
 * If the coordinate is outside the puzzle, nullptr is returned.
 * @param coord         Coordinate
 * @return const pointer to cell, if existing
 */
ConstCellPtr Puzzle::getConstCellAtCoordinate (Coordinate c) const noexcept
{
    //if (!passCoordinateRangeCheck(c))
        //return nullptr;
    if (c[0] >= m_puzzleRows.size())
        return nullptr;
    if (c[1] >= m_puzzleRows[c[0]].size())
        return nullptr;
    return m_puzzleRows[c[0]][c[1]];
}

/**
 * From the given coordinate determine how many cells can be traversed before reaching
 * an obstruction. The obstruction can be a wall or any pipe.
 * @param c     Starting coordinate
 * @param d     Traversal direction
 * @return number of empty cells between c and obstruction
 */
unsigned Puzzle::gapToObstruction (Coordinate c, Direction d) const noexcept
{
    unsigned count = 0;
    ConstCellPtr pCell = getConstCellAtCoordinate(c);
    while (1)
    {
        // check if cell has a border in that direction.
        if (pCell->getBorder(d) == CellBorder::WALL)
            break;
        if (!coordinateChange(c, d))
            break;
        pCell = getConstCellAtCoordinate(c);
        if (!pCell->isEmpty())
            break;
        ++count;
    }
    return count;
}

/**
 * Check whether set of routes is a puzzle solution.
 */
bool Puzzle::checkIfSolution (ConstPuzzlePtr puzzle, const std::map<PipeId, Route> & s)
{
    if (s.size() != puzzle->getNumPipes())
        return false; // Require one route per pipe

    std::set<Coordinate> coordinates;
    for (auto p : s)
    {
        for (const Coordinate & coord : p.second)
        {
            if (coordinates.find(coord) != coordinates.end())
                return false; // Coordinate repeated. So given routes are not a solution.
            coordinates.insert(coord);
        }
    }
    // Check every cell is used.
    for (unsigned r = 0; r < puzzle->getNumRows(); ++r)
    {
        for (unsigned c = 0; c < puzzle->getNumCols(); ++c)
        {
            if (!puzzle->isCellReachable({r, c}))
                continue;
            if (coordinates.find({r, c}) == coordinates.end()) // TODO change to simple number, rather than iterate over cells
                return false;
        }
    }
    logger << "Solution found:" << std::endl;
    for (std::pair<PipeId, Route> p : s)
    {
        logger << p.first << ": " << p.second << std::endl;
        for (Coordinate coord : p.second)
            puzzle->m_puzzleRows[std::get<0>(coord)][std::get<1>(coord)]->setPipeId(p.first);
    }
    puzzle->streamPuzzleMatrix(std::cout);
    return true;
}

// Adjacent using Adjacency type ---------------------------------------------

/**
 * Get the adjacent cell in the given direction, regardless of inner walls.
 * (Inner walls are disregarded because diagonally has no direct route.
 * So keep result consistent for all adjacent.)
 * @param direction     Direction from start coordinate to lookup
 * @param c     Coordinate of start cell
 * @return adjacent cell, regardless of inner walls. If an outer wall is in the direction, return nullptr.
 */
ConstCellPtr Puzzle::getConstCellAdjacent (Coordinate c, Adjacency direction) const //noexcept
{
    if (!passCoordinateRangeCheck(c))
        return nullptr; // Invalid start coordinate
    if (!coordinateChange(c, direction))
        return nullptr;
    return getConstCellAtCoordinate(c);
}

/**
 * Get the adjacent cell in the given direction.
 * (Inner walls are disregarded because diagonally has no direct route.
 * So keep result consistent for all adjacent.)
 * @param direction     Direction from start coordinate to lookup
 * @param c     Coordinate of start cell
 * @return adjacent cell, regardless of inner walls. If an outer wall is in the direction, return nullptr.
 */
CellPtr Puzzle::getCellAdjacent (Coordinate c, Adjacency direction) //noexcept
{
    return std::const_pointer_cast<Cell>(getConstCellAdjacent(c, direction));
}

// Adjacent using Direction type ---------------------------------------------

/**
 * Return the cell adjacent to the given coordinate in the given direction,
 * only if it is immediately reachable in that direction. Disregards pipes.
 * ie. if not blocked by a wall
 * @param coord         Current coordinate
 * @param d     Direction to adjacent cell
 * @return adjacent Cell if direction is open, or nullptr if the direction is blocked by a wall.
 */
CellPtr Puzzle::getCellAdjacent (Coordinate c, Direction d) //noexcept
{
    return std::const_pointer_cast<Cell>(getConstCellAdjacent(c, d));
}

/**
 * Return the cell adjacent to the given coordinate in the given direction,
 * only if it is immediately reachable in that direction. Disregards pipes.
 * ie. if not blocked by a wall
 * @param c     Current coordinate
 * @param d     Direction to adjacent cell
 * @return adjacent Cell if direction is open, or nullptr if the direction is blocked by a wall.
 */
ConstCellPtr Puzzle::getConstCellAdjacent (Coordinate c, Direction d) const //noexcept
{
    if (!passCoordinateRangeCheck(c))
        return nullptr; // Invalid start coordinate
    const ConstCellPtr pCell = getConstCellAtCoordinate(c);
    if (d == Direction::NONE)
        return pCell;
    if (pCell->isBorderOpen(d) && coordinateChange(c, d))
        return getConstCellAtCoordinate(c);
    return nullptr;
}

/**
 * Get the cells adjacent to the given one.
 * @return adjacent cells mapped by direction. The direction is from the cell towards the adjacement.
 */
std::map<Direction, ConstCellPtr> Puzzle::getAdjacentCells (ConstCellPtr cell) const noexcept
{
    std::map<Direction, ConstCellPtr> result;
    if (cell == nullptr)
        return result;

    for (Direction d : allTraversalDirections)
    {
        ConstCellPtr p = getConstCellAdjacent(cell->getCoordinate(), d);
        if (p != nullptr)
            result[d] = p;
    }
    return result;
}

/**
 * Get all cells surrounding the given coordinate. Disregards inner walls.
 */
std::array<ConstCellPtr, 9> Puzzle::getAdjacentCells (Coordinate coord) const //noexcept
{
    std::array<ConstCellPtr, 9> result;
    result[Adjacency::ADJACENT_NORTH_WEST] = (isCoordinateChangeValid(coord, Adjacency::ADJACENT_NORTH_WEST) ?
            getConstCellAdjacent(coord, Adjacency::ADJACENT_NORTH_WEST) : nullptr);
    result[Adjacency::ADJACENT_NORTH] = (isCoordinateChangeValid(coord, Adjacency::ADJACENT_NORTH) ?
            getConstCellAdjacent(coord, Adjacency::ADJACENT_NORTH) : nullptr);
    result[Adjacency::ADJACENT_NORTH_EAST] = (isCoordinateChangeValid(coord, Adjacency::ADJACENT_NORTH_EAST) ?
            getConstCellAdjacent(coord, Adjacency::ADJACENT_NORTH_EAST) : nullptr);
    result[Adjacency::ADJACENT_WEST] = (isCoordinateChangeValid(coord, Adjacency::ADJACENT_WEST) ?
            getConstCellAdjacent(coord, Adjacency::ADJACENT_WEST) : nullptr);
    result[Adjacency::ADJACENT_CENTRAL] =
            getConstCellAtCoordinate(coord);
    result[Adjacency::ADJACENT_EAST] = (isCoordinateChangeValid(coord, Adjacency::ADJACENT_EAST) ?
            getConstCellAdjacent(coord, Adjacency::ADJACENT_EAST) : nullptr);
    result[Adjacency::ADJACENT_SOUTH_WEST] = (isCoordinateChangeValid(coord, Adjacency::ADJACENT_SOUTH_WEST) ?
            getConstCellAdjacent(coord, Adjacency::ADJACENT_SOUTH_WEST) : nullptr);
    result[Adjacency::ADJACENT_SOUTH] = (isCoordinateChangeValid(coord, Adjacency::ADJACENT_SOUTH) ?
            getConstCellAdjacent(coord, Adjacency::ADJACENT_SOUTH) : nullptr);
    result[Adjacency::ADJACENT_SOUTH_EAST] = (isCoordinateChangeValid(coord, Adjacency::ADJACENT_SOUTH_EAST) ?
            getConstCellAdjacent(coord, Adjacency::ADJACENT_SOUTH_EAST) : nullptr);
    return result;
}

/**
 * Insert the route into the puzzle.
 * Does not change fixture status of any cell.
 * The puzzle will remember the additions, and will remove them in removeRoute()
 */
void Puzzle::insertRoute (PipeId idPipe, const Route & route)
{
    for (Coordinate coord : route)
    {
        CellPtr pCell = getCellAtCoordinate(coord);
        if (pCell->getPipeId() != idPipe)
        {
            if (pCell->getPipeId() != NO_PIPE_ID)
            {
                // unexpected pipe mismatch
                throw PuzzleException("attempt to insert route on top of another");
            }
            // else new pipe component
            pCell->setPipeId(idPipe);
            m_injectedRoute.insert(coord);
        }
        // else no change to cell already in route
    }
}

/**
 * Remove route that was previously inserted.
 * Does not remove the elements of the route that existed prior.
 * (ie. does not remove fixtures)
 */
void Puzzle::removeRoute ()
{
    for (Coordinate coord : m_injectedRoute)
    {
        CellPtr pCell = getCellAtCoordinate(coord);
        pCell->setPipeId(NO_PIPE_ID);
    }
    m_injectedRoute.clear();
}

// TODO make this part of Puzzle?? Would need some local static functions moved into class too.
static std::set<Coordinate> visited;

/**
 * If the route can continue from the given cell in the given direction,
 * then add the next coordinate in that direction to the route.
 * @return true if the route can go in direction from cell
 */
static bool continueDirectionForRoute (ConstCellPtr pCell, Route & route, Direction direction)
{
    Coordinate coord = pCell->getCoordinate();
    if (!coordinateChange(coord, direction))
        return false;
    if (pCell->isConnected(direction) && visited.find(coord) == visited.end())
    {
        logger << " to " << coord;
        route.push_back(coord);
        return true;
    }
    return false;
}

/**
 * Trace routes for pipe in current puzzle state.
 * @return true if complete route found
 */
bool Puzzle::traceRoute (PipeId idPipe, Route & route) const
{
    if (!route.size())
    {
        // start
        Coordinate coordStart = findPipeEnd(idPipe, PipeEnd::PIPE_END_1);
        logger << "Trace " << idPipe << " from " << coordStart;
        visited.clear();
        route.push_back(coordStart);
    }
    Coordinate coord = route.back();
    visited.insert(coord);
    ConstCellPtr pCell = getConstCellAtCoordinate(coord);
    if (pCell->getEndpoint() == PipeEnd::PIPE_END_2)
    {
        logger << " endpoint." << std::endl;
        return true;
    }
    //std::bind();
#if 0
    // lambda to check connection, and next coordinate (in given direction) to route, if not already visited
    auto lam = [&route, pCell, coord, idPipe] (Direction direction) -> bool {
        return continueDirectionForRoute(pCell, route, direction);
    };
    forEachDirection(lam);
#endif
    //std::find(std::begin(visited), std::end(visited), coordinateChange()) != visited.end;
    if (continueDirectionForRoute(pCell, route, Direction::UP))
        return traceRoute(idPipe, route);
    else if (continueDirectionForRoute(pCell, route, Direction::DOWN))
        return traceRoute(idPipe, route);
    else if (continueDirectionForRoute(pCell, route, Direction::LEFT))
        return traceRoute(idPipe, route);
    else if (continueDirectionForRoute(pCell, route, Direction::RIGHT))
        return traceRoute(idPipe, route);
    logger << " stopped." << std::endl;
    return false;
}

void Puzzle::traceRoutes (std::map<PipeId, Route> & m) const
{
    logger << "Trace routes" << std::endl;
    Route route = {};
    m.clear();
    for (PipeId idPipe : m_def.getPipeIds())
    {
        traceRoute(idPipe, route);
        m[idPipe] = route;
    }
}
