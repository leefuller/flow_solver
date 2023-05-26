#include "Helper.h"

#include "../include/Cell.h"
#include "../include/Puzzle.h"

/**
 * Check whether a cell is a corner, defined by the walls.
 * Disregards pipes.
 * @param coord     Coordinate of cell to check
 * @return true if the cell at the given coordinate is a corner
 */
bool Helper::isCorner (ConstPuzzlePtr puzzle, Coordinate coord) noexcept
{
    ConstCellPtr pCell = puzzle->getConstCellAtCoordinate(coord);
    // A corner has 2 connected walls that are not opposite
    if (pCell->countWalls() != 2)
        return false;
    return !pCell->isHorizontalChannel() && !pCell->isVerticalChannel();
}

/**
 * Determine obstructions at the given cell. Add obstructed directions to the given walls and pipes.
 * @param puzzle    Puzzle to assess
 * @param pCell     Cell to check for obstructions
 * @param walls     To return directions to adjacent walls.
 * @param pipes     To return directions to adjacent pipes.
 * @return number of obstructions
 */
unsigned Helper::getObstructedDirections (ConstPuzzlePtr puzzle, ConstCellPtr pCell, std::set<Direction> & walls, std::set<Direction> & pipes) noexcept
{
    unsigned count = 0;
    for (Direction d : allTraversalDirections)
    {
        if (pCell->getBorder(d) == CellBorder::WALL)
        {
            walls.insert(d);
            ++count;
        }
        else // check if obstructed by pipe
        {
            for (Direction d : allTraversalDirections)
            {
                ConstCellPtr pCellAdjacent = puzzle->getConstCellAdjacent(pCell->getCoordinate(), d);
                if (pCellAdjacent == nullptr) // no cell adjacent
                    continue; // No addition to count, because there should be a wall that was counted above
                if (pCellAdjacent->getPipeId() != NO_PIPE_ID)
                {
                    pipes.insert(d);
                    ++count;
                }
            }
        }
    }
    return count;
}

/**
 * Gets empty cells from the given coordinate in the given direction until coming to a wall or non-empty cell.
 * Result includes the cell at the given coordinate, if it is empty.
 * ie. If the cell at the coordinate is not empty, the result is an empty list.
 */
std::vector<ConstCellPtr> Helper::getCellsUntilObstruction(ConstPuzzlePtr puzzle, Coordinate c, Direction d) noexcept
{
    std::vector<ConstCellPtr> result;

    if (puzzle->passCoordinateRangeCheck(c))
    {
        while (true)
        {
            ConstCellPtr pCell = puzzle->getConstCellAtCoordinate(c);
            if (pCell == nullptr)
                break;
            if (pCell->getPipeId() != NO_PIPE_ID)
                break;
            result.push_back(pCell);
            if (!coordinateChange(c, d))
                break;
        }
    }
    return result;
}

/**
 * Get all traversable directions from coordinate.
 * Traversal is prevented by WALL, or if the target is already occupied,
 * unless the occupied target is the pipe endpoint.
 * Does not check if endpoint has another connection.
 *
 * @return all traversable directions from coordinate.
 */
std::set<Direction> Helper::getNowTraversableDirections (ConstPuzzlePtr puzzle, Coordinate coord, PipeId idPipe)
{
    std::set<Direction> result;
    for (Direction d : allTraversalDirections)
    {
        if (canNowTraverseDirectionFrom(puzzle, coord, d, idPipe))
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
bool Helper::canNowTraverseDirectionFrom (ConstPuzzlePtr puzzle, Coordinate coord, Direction direction, PipeId idPipe) noexcept
{
    ConstCellPtr pCellFrom = puzzle->getConstCellAtCoordinate(coord);
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
    ConstCellPtr pCellNext = puzzle->getConstCellAtCoordinate(nextCoord);

    if (idPipe == NO_PIPE_ID)
        return true;
    if (pCellNext->getPipeId() == idPipe && pCellNext->isEndpoint())
        return true;
    return pCellNext->isEmpty(); // can traverse to empty cell
}
