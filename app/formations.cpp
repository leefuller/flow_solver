#include "Puzzle.h"
#include "Cell.h"
#include "PuzzleException.h"
#include "formations.h"
#include "Logger.h"

#include <array>
#include <algorithm>
#include <optional>

static Logger & logger = Logger::getDefaultLogger();

/**
 * Check whether a cell is a corner, defined by the walls.
 * Disregards pipes.
 * @param coord     Coordinate of cell to check
 * @return true if the cell at the given coordinate is a corner
 */
bool isCorner (ConstPuzzlePtr puzzle, Coordinate coord) noexcept
{
    ConstCellPtr pCell = puzzle->getConstCellAtCoordinate(coord);
    // A corner has 2 connected walls that are not opposite
    if (pCell->countWalls() != 2)
        return false;
    return !pCell->isHorizontalChannel() && !pCell->isVerticalChannel();
}

/**
 * Check whether a cell at a coordinate is on the inside of a corner,
 * where the corner is created by walls.
 * A corner exists where a cell is immediately obstructed in 2 traversal directions that are 90 degrees separated.
 * Whether or not the cell contains a pipe is disregarded.
 * Here are the 4 formations:
 *
 *    =      =      |.     .|
 *   |.      .|      =     =
 *
 * The direction of a corner is always a diagonal one.
 * @return the direction to the corner, or Direction::NONE if there is not a corner
 */
Direction checkCornerAtCoordinate (ConstPuzzlePtr puzzle, Coordinate c)
{
    return cornerDirection(puzzle->getGapsToWalls(c));
}

/**
 * Check whether a cell at a coordinate is 1 cell separated from a corner,
 * without a wall between. A pipe can be between.
 * (Think of it as though you cannot see through a wall, but you can see past a pipe.)
 * For example:
 *
 *   . X    The cell 'X' is at a south-west corner, south-west of 'X'
 *  |. .    So this function will return SOUTH_WEST if the direction parameter is SOUTH_WEST,
 *   ==     and there is no inner wall
 *
 *          The cell 'X' is at a south-west corner
 *  |. X    So this function will return SOUTH_WEST if the direction parameter is WEST,
 *   ==     and there is no inner wall
 *
 * @param c     Cell coordinate
 * @param d     Direction to "look" from the coordinate
 * @return direction of corner, or Direction::NONE if there is not a corner in the given direction
 */
Direction checkOneStepToCorner (ConstPuzzlePtr puzzle, Coordinate c, Direction d)
{
    ConstCellPtr p = puzzle->getConstCellAtCoordinate(c);
    if (p == nullptr)
        return Direction::NONE;
    // Need to check adjacent cell for a corner-1 formation
    ConstCellPtr pAdj = puzzle->getConstCellAdjacent(c, d); // disregards walls
    if (pAdj == nullptr)
        return Direction::NONE;

    Direction dc = checkCornerAtCoordinate(puzzle, pAdj->getCoordinate());
    switch (dc)
    {
        case Direction::NORTH_EAST:
            /* North-east corner visible for 3 cases
                    (1)      (2)      (3)
                    ==        ==       ==
                    .|      X .|       .|
                    X                    X

                The direction to "look" must see the inside of the corner
                */
            if (d == Direction::NORTH_EAST) // case 1
                return (p->isBorderOpen(Direction::NORTH) && p->isBorderOpen(Direction::EAST)) ? dc : Direction::NONE;
            if (d == Direction::EAST) // case 2
                return p->isBorderOpen(Direction::EAST) ? dc : Direction::NONE;
            if (d == Direction::NORTH) // case 3
                return p->isBorderOpen(Direction::NORTH) ? dc : Direction::NONE;
            return Direction::NONE;

        case Direction::NORTH_WEST:
            /* North-west corner visible for 3 cases
                    (1)        (2)        (3)
                    ==          ==         ==
                |.          |. X       |.
                    X                    X

                The direction to "look" must see the inside of the corner
                */
            if (d == Direction::NORTH_WEST) // case 1
                return (p->isBorderOpen(Direction::NORTH) && p->isBorderOpen(Direction::WEST)) ? dc : Direction::NONE;
            if (d == Direction::WEST) // case 2
                return p->isBorderOpen(Direction::WEST) ? dc : Direction::NONE;
            if (d == Direction::NORTH) // case 3
                return p->isBorderOpen(Direction::NORTH) ? dc : Direction::NONE;
            return Direction::NONE;

        case Direction::SOUTH_EAST:
            /* South-east corner visible for 3 cases
                    (1)      (2)      (3)
                    X                    X
                    .|      X .|       .|
                    ==        ==       ==

                The direction to "look" must see the inside of the corner
                */
            if (d == Direction::SOUTH_EAST) // case 1
                return (p->isBorderOpen(Direction::SOUTH) && p->isBorderOpen(Direction::EAST)) ? dc : Direction::NONE;
            if (d == Direction::EAST) // case 2
                return p->isBorderOpen(Direction::EAST) ? dc : Direction::NONE;
            if (d == Direction::SOUTH) // case 3
                return p->isBorderOpen(Direction::SOUTH) ? dc : Direction::NONE;
            return Direction::NONE;

        case Direction::SOUTH_WEST:
            /* South-west corner visible for 3 cases
                    (1)      (2)      (3)
                    X               X
                    |.        |. X    |.
                    ==        ==      ==

                The direction to "look" must see the inside of the corner
                */
            if (d == Direction::SOUTH_WEST) // case 1
                return (p->isBorderOpen(Direction::SOUTH) && p->isBorderOpen(Direction::WEST)) ? dc : Direction::NONE;
            if (d == Direction::WEST) // case 2
                return p->isBorderOpen(Direction::WEST) ? dc : Direction::NONE;
            if (d == Direction::SOUTH) // case 3
                return p->isBorderOpen(Direction::SOUTH) ? dc : Direction::NONE;
            return Direction::NONE;
    }
    return Direction::NONE;
}

/**
 * @param puzzle    The puzzle state
 * @param coord     Coordinate for cell
 * @param idPipe    Pipe identifier
 * @param inwardDirection   The side from which the cell is entered.
 * @return true if a deviation has formed that would invalidate any route possible there
 */
bool detectInvalidDeviation (ConstPuzzlePtr puzzle, const Route & route, PipeId idPipe)
{
/*
 * Need to eliminate a route like this:
 *
 * X X . . X        Where a different route using the 2 empty cells would be eliminated by the adjacency rule.
 *   X X X X        Detection of the situation in an earlier route means less processing for later routes.
 *                  Similarly in all other directions.
 * Note if either cell in the "gap" is an endpoint different to route X, the route is still valid.
 * Then, the cell next to it must belong to the same other route, otherwise there is dead end there, for any other pipe.
 * (As would be detected by "the only way" algorithm)
 *
 * Also, "depth" of the gap does not matter. A formation like this is the same issue:
 * X X . . X
 *   X . . X
 *   X X X X
 * An endpoint anywhere within this means the route is still invalid, because there is a dead end,
 * or the adjacency rule would be broken.
 *
 * A deviation like this one creates a dead end, for which there is a separate rule:
 * X X . X
 *   X X X
 *
 * A deviation like this one is valid:
 * X X . . . X
 *   X X X X X
 * There is enough space for another route to enter and leave the 3 cell gap without violating the adjacency rule.
 */
    // TODO implement

    return false;
}

/**
 * Detect whether the given route results in a bad formation.
 * @param puzzle    Puzzle to assess
 * @param route     Route to assess
 * @param idPipe    Pipe identifier for route
 */
bool detectBadFormation (ConstPuzzlePtr puzzle, const Route & route, PipeId idPipe)
{
    try
    {
        if (adjacencyRuleBroken(puzzle, route))
        {
#if ANNOUNCE_ADJACENCY_LAW_BREAK
            logger << "Adjacency rule broken for " << idPipe << " route " << route << std::endl;
            Cell::setOutputConnectorRep(false);
            puzzle->streamPuzzleMatrix(logger.stream());
#endif
            return true;
        }
    }
    catch (const PuzzleException & ex)
    {
#if ANNOUNCE_SOLVER_DETAIL
        logger << "Puzzle exception in checking adjacency for route: " << ex << std::endl;
#endif
        throw;
    }
    try
    {
        if (detectDeadEndFormation(puzzle, route, idPipe))
        {
#if ANNOUNCE_DEAD_END_DETECT
            logger << "Dead end formation for " << idPipe << " in route " << route << std::endl;
            Cell::setOutputConnectorRep(false);
            puzzle->streamPuzzleMatrix(logger.stream());
#endif
            return true;
        }
    }
    catch (const PuzzleException & ex)
    {
#if ANNOUNCE_SOLVER_DETAIL
        logger << "Puzzle exception in checking for dead end formation for route: " << ex << std::endl;
#endif
        throw;
    }
    try
    {
        if (detectInvalidDeviation(puzzle, route, idPipe))
        {
#if ANNOUNCE_INVALID_DEVIATION
            logger << "Invalid deviation for " << idPipe << " in route " << route << std::endl;
            Cell::setOutputConnectorRep(false);
            puzzle->streamPuzzleMatrix(logger.stream());
#endif
            return true;
        }
    }
    catch (const PuzzleException & ex)
    {
#if ANNOUNCE_SOLVER_DETAIL
        logger << "Puzzle exception in checking for invalid deviation for route: " << ex << std::endl;
#endif
        throw;
    }
    if (detectEntrapment(puzzle, route, idPipe))
    {
#if ANNOUNCE_ENTRAPMENT
        logger << "Entrapment due to " << idPipe << " route " << route << std::endl;
        Cell::setOutputConnectorRep(false);
        puzzle->streamPuzzleMatrix(logger.stream());
#endif
        return true;
    }

    return false;
}
