#include "Puzzle.h"
#include "PuzzleException.h"
#include "Cell.h"
#include "Logger.h"

#include <memory>
#include <charconv> // to_chars
#include <iostream>
#include <map>

static Logger & logger = Logger::getDefaultLogger();

/**
 * Create new Puzzle based on given definition.
 *
 * As a convenience, if a cell is an start/end point,
 * the set of possibilities for the cell is set to only contain the one in the definition.
 * No other cells have that attribute set at creation.
 *
 * @param def   Definition from which to derive puzzle
 */
Puzzle::Puzzle (const PuzzleDefinition & def)
    : m_def(def), m_puzzleRows(def.generateRows())
{}

/**
 * Perform a deep copy.
 * ie. The pointers to cells in the new puzzle do not point into cells in p
 */
Puzzle::Puzzle (const Puzzle & p) :
    m_def(p.m_def)
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
 * Get the cell at the given coordinate.
 * If the coordinate is outside the puzzle, nullptr is returned.
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
    if (!passCoordinateRangeCheck(c))
        return nullptr;
    return m_puzzleRows[c[0]][c[1]];
}

/**
 * From the given coordinate determine how many cells can be traversed before reaching
 * an obstruction. The obstruction can be a wall or any pipe. The pipe does not need to be a fixture.
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
 * Convenience function return the gapToObstruction function result for each each traversal direction from a coordinate.
 * An obstruction can be a wall or any pipe. The pipe does not need to be a fixture.
 * @return array indexed by Direction
 */
std::array<unsigned, 4> Puzzle::getGapsToObstructions (Coordinate c) const noexcept
{
    std::array<unsigned, 4> result;
    for (Direction d : allTraversalDirections)
        result[d] = gapToObstruction(c, d);
    return result;
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
    for (int r = 0; r < puzzle->getNumRows(); ++r)
    {
        for (int c = 0; c < puzzle->getNumCols(); ++c)
        {
            if (!puzzle->isCellReachable({r, c}))
                continue;
            if (coordinates.find({r, c}) == coordinates.end()) // TODO change to simple number, rather than iterate over cells
                return false;
        }
    }
    logger << "Solution found:" << std::endl;
    Cell::setOutputConnectorRep(false);
    for (std::pair<PipeId, Route> p : s)
    {
        logger << p.first << ": " << p.second << std::endl;
        for (Coordinate coord : p.second)
            puzzle->m_puzzleRows[std::get<0>(coord)][std::get<1>(coord)]->setPipeId(p.first);
    }
    puzzle->streamPuzzleMatrix(std::cout);
    return true;
}

/**
 * Get all traversable directions from coordinate.
 * Traversal is prevented by WALL, or if the target is already occupied,
 * unless the occupied target is the pipe endpoint.
 * Does not check if endpoint has another connection.
 *
 * @return all traversable directions from coordinate.
 */
std::set<Direction> Puzzle::getNowTraversableDirections (Coordinate coord, PipeId idPipe) const
{
    std::set<Direction> result;
    for (Direction d : allTraversalDirections)
    {
        if (canNowTraverseDirectionFrom(coord, d, idPipe))
            result.insert(d);
    }
    return result;
}

/**
 * Return true if nothing prevents traversal in given direction from given coordinate.
 * Traversal is prevented by WALL, or if the target is already occupied,
 * unless the occupied target is the pipe endpoint.
 * @param direction     Direction from start coordinate to check for traversal
 * @param coord         Start coordinate
 * @param idPipe        Pipe identifier. Can be NO_PIPE_ID to check disregarding pipes (TODO remove this complexity).
 */
bool Puzzle::canNowTraverseDirectionFrom (Coordinate coord, Direction direction, PipeId idPipe) const noexcept
{
    ConstCellPtr pCellFrom = getConstCellAtCoordinate(coord);
    if (!pCellFrom->isBorderOpen(direction))
        return false;

    if (pCellFrom->getConnection(direction) == CellConnection::FIXTURE_CONNECTION)
        return false;

    if (pCellFrom->isEndpoint())
    {
        // Cannot traverse if the endpoint is already connected to a fixture.
        // There should be no connector in any direction other than the fixture.
        for (Direction d : allTraversalDirections)
        {
            if (pCellFrom->getConnection(d) == CellConnection::FIXTURE_CONNECTION)
                return false; // endpoint already has a fixture connection
        }
    }
    else
    {
        // Cannot traverse if not an endpoint and there are 2 fixtures to the cell.
        unsigned count = 0;
        for (Direction d : allTraversalDirections)
        {
            if (pCellFrom->getConnection(d) == CellConnection::FIXTURE_CONNECTION)
                ++count;
        }
        if (count == 2)
            return false;
    }


    Coordinate nextCoord = coord;
    if (!coordinateChange(nextCoord, direction))
        return false;

    // At this point we know a cell exists in the given direction, and there is no wall preventing traversal.
    ConstCellPtr pCellNext = getConstCellAtCoordinate(nextCoord);

    if (idPipe == NO_PIPE_ID)
        return true;
    if (pCellNext->getPipeId() == idPipe && pCellNext->isEndpoint())
        return true;
    return pCellNext->isEmpty(); // can traverse to empty cell
}

// Adjacent -----------------------------------------------------------------

/**
 * Get the adjacent cell in the given direction, regardless of inner walls.
 * (Inner walls are disregarded because diagonally has no direct route.
 * So keep result consistent for all directions.)
 * @param direction     Direction from start coordinate to lookup
 * @param c     Coordinate of start cell
 * @return adjacent cell, regardless of inner walls. If an outer wall is in the direction, return nullptr.
 */
ConstCellPtr Puzzle::getConstCellAdjacent (Coordinate c, Direction direction) const //noexcept
{
    if (!passCoordinateRangeCheck(c))
        return nullptr; // Invalid start coordinate
    if (!coordinateChange(c, direction))
        return nullptr;
    return getConstCellAtCoordinate(c);
}

/**
 * Get the adjacent cell in the given direction, regardless of inner walls.
 * (Inner walls are disregarded because diagonally has no direct route.
 * So keep result consistent for all directions.)
 * @param direction     Direction from start coordinate to lookup
 * @param c     Coordinate of start cell
 * @return adjacent cell, regardless of inner walls. If an outer wall is in the direction, return nullptr.
 */
CellPtr Puzzle::getCellAdjacent (Coordinate c, Direction direction) //noexcept
{
    return std::const_pointer_cast<Cell>(getConstCellAdjacent(c, direction));
}

/**
 * Get all cells surrounding the given coordinate. Disregards inner walls.
 * Returned element will contain nullptr for any direction where there is no cell.
 */
std::map<Direction, ConstCellPtr> Puzzle::getSurroundingCells (Coordinate coord) const //noexcept
{
    std::map<Direction, ConstCellPtr> result;
    result[Direction::NORTH_WEST] = (isCoordinateChangeValid(coord, Direction::NORTH_WEST) ?
            getConstCellAdjacent(coord, Direction::NORTH_WEST) : nullptr);
    result[Direction::NORTH] = (isCoordinateChangeValid(coord, Direction::NORTH) ?
            getConstCellAdjacent(coord, Direction::NORTH) : nullptr);
    result[Direction::NORTH_EAST] = (isCoordinateChangeValid(coord, Direction::NORTH_EAST) ?
            getConstCellAdjacent(coord, Direction::NORTH_EAST) : nullptr);
    result[Direction::WEST] = (isCoordinateChangeValid(coord, Direction::WEST) ?
            getConstCellAdjacent(coord, Direction::WEST) : nullptr);
    result[Direction::EAST] = (isCoordinateChangeValid(coord, Direction::EAST) ?
            getConstCellAdjacent(coord, Direction::EAST) : nullptr);
    result[Direction::SOUTH_WEST] = (isCoordinateChangeValid(coord, Direction::SOUTH_WEST) ?
            getConstCellAdjacent(coord, Direction::SOUTH_WEST) : nullptr);
    result[Direction::SOUTH] = (isCoordinateChangeValid(coord, Direction::SOUTH) ?
            getConstCellAdjacent(coord, Direction::SOUTH) : nullptr);
    result[Direction::SOUTH_EAST] = (isCoordinateChangeValid(coord, Direction::SOUTH_EAST) ?
            getConstCellAdjacent(coord, Direction::SOUTH_EAST) : nullptr);
    return result;
}

/**
 * Get adjacent cells in traversal directions for the given coordinate.
 * Returned element will contain nullptr for any direction where there is no cell, and,
 * for the case where walls are blockers, if the cell is obstructed by a wall.
 * @param coord
 * @param wallsBlock    If true, do not include cells obstructed by a wall
 */
std::map<Direction, ConstCellPtr> Puzzle::getAdjacentCellsInTraversalDirections (Coordinate coord, bool wallsBlock) const //noexcept
{
    std::map<Direction, ConstCellPtr> result;
    if (!wallsBlock)
    {
        for (Direction d : allTraversalDirections)
        {
            result[d] = (isCoordinateChangeValid(coord, d) ?
                    getConstCellAdjacent(coord, d) : nullptr);
        }
    }
    else
    {
        ConstCellPtr p = getConstCellAtCoordinate(coord);
        for (Direction d : allTraversalDirections)
        {
            result[d] = ((isCoordinateChangeValid(coord, d) && p->isBorderOpen(d)) ?
                    getConstCellAdjacent(coord, d) : nullptr);
        }
    }
    return result;
}

bool Puzzle::isProxyEnd (PipeId id, Coordinate c) const noexcept
{
    Route route;
    traceRoute(id, PipeEnd::PIPE_END, route);
    return route.back() == c;
}

// ----------------------------------------------------

/**
 * Insert the route into the puzzle.
 * Does not change fixture status of any cell.
 * The puzzle will remember the additions, and will remove them in removeRoute()
 * @param idPipe    Pipe identifier
 * @param route     Route to insert
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

/**
 * If the route can continue from the given cell in the given direction,
 * then add the next coordinate in that direction to the route.
 * @param pCell     
 * @param route     Route being created
 * @param direction Direction from pCell
 * @return true if the route can go in direction from cell
 */
bool Puzzle::continueDirectionForRoute (ConstCellPtr pCell, Route & route, Direction direction) const noexcept
{
    Coordinate coord = pCell->getCoordinate();
    if (!coordinateChange(coord, direction))
        return false;
    if (pCell->isConnected(direction) && m_visited.find(coord) == m_visited.end())
    {
        route.push_back(coord);
        return true;
    }
    return false;
}

/**
 * Trace route for a pipe in current puzzle state.
 * @param idPipe    Pipe identifier
 * @param endpoint  Where to start the trace (PIPE_START for forward trace from start, or PIPE_END to trace back from end)
 * @param route     To return route traced from start of pipe
 * @return true if complete route found
 */
bool Puzzle::traceRoute (PipeId idPipe, PipeEnd endpoint, Route & route) const
{
    if (!route.size())
    {
        Coordinate coordStart = findPipeEnd(idPipe, endpoint);
        m_visited.clear();
        route.push_back(coordStart);
    }
    Coordinate coord = route.back();
    m_visited.insert(coord);
    ConstCellPtr pCell = getConstCellAtCoordinate(coord);
    if (pCell->getEndpoint() == oppositeEnd(endpoint))
        return true;

    if (continueDirectionForRoute(pCell, route, Direction::NORTH))
        return traceRoute(idPipe, endpoint, route);
    else if (continueDirectionForRoute(pCell, route, Direction::SOUTH))
        return traceRoute(idPipe, endpoint, route);
    else if (continueDirectionForRoute(pCell, route, Direction::WEST))
        return traceRoute(idPipe, endpoint, route);
    else if (continueDirectionForRoute(pCell, route, Direction::EAST))
        return traceRoute(idPipe, endpoint, route);
    return false;
}

/**
 * Trace routes for all pipes.
 */
void Puzzle::traceRoutes (std::map<PipeId, Route> & m) const
{
    logger << "Trace routes" << std::endl;
    Route route = {};
    m.clear();
    for (PipeId idPipe : m_def.getPipeIds())
    {
        traceRoute(idPipe, PipeEnd::PIPE_START, route);
        m[idPipe] = route;
    }
}
