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
 * Find a pipe endpoint that may be reachable via empty cells only.
 * @param puzzle
 * @param c         Start coordinate
 * @param idPipe    Identifier of pipe
 * @param visited   Coordinates visited so far
 * @return true if the endpoint is found.
 */
static bool recurseReachable (ConstPuzzlePtr puzzle, Coordinate c, PipeId idPipe, std::set<Coordinate> & visited)
{
    for (Direction d : allTraversalDirections)
    {
        if (!puzzle->isCoordinateChangeValid (c, d))
            continue;

        ConstCellPtr pAdj = puzzle->getConstCellAdjacent(c, d);
        if (pAdj == nullptr)
            continue;
        if (std::find(std::begin(visited), std::end(visited), pAdj->getCoordinate()) != visited.end()) // already visited
            continue;
        if (!pAdj->isEmpty())
        {
            if (pAdj->getPipeId() == idPipe && (pAdj->getEndpoint() == PipeEnd::PIPE_END || puzzle->isProxyEnd(idPipe, pAdj->getCoordinate())))
            {
                // found
                //logger << "Reachable end found for pipe " << idPipe << " at " << pAdj->getCoordinate() << std::endl;
                return true;
            }
            continue;
        }
        visited.insert(pAdj->getCoordinate());
        if (recurseReachable(puzzle, pAdj->getCoordinate(), idPipe, visited))
            return true;
    }
    return false;
}

/**
 * Entrapment exists where a route traps empty cells,
 * or cells that have one endpoint for a particular pipe, but not both,
 * within an area that prevents any other pipe from reaching.
 * @param puzzle    The puzzle state being assessed
 * @param route     The route being considered
 * @param idPipe    The pipe id for the route
 * @return true if the route traps something
 */
bool detectEntrapment (ConstPuzzlePtr puzzle, const Route & route, PipeId idPipe)
{
    // Note, there can be more than 1 trapped area. Return true on detection of first.
    // An area trapped could be a lot of the puzzle.
    // An endpoint is trapped if it cannot reach the corresponding endpoint, where the route acts as a wall.

    bool trapped = false;

    // The simplest case is where a single empty cell is trapped.
    // (ie. on each side is a border or other pipe.)
    std::function<void(ConstCellPtr)> lam = [&puzzle, &trapped, idPipe](ConstCellPtr cell){
        if (!puzzle->isCellReachable(cell->getCoordinate()))
            return;
        // Dead end formation can indicate a cell is trapped
        if (detectDeadEndFormation(puzzle, cell->getCoordinate()))
        {
            logger << "Trap at dead end " << cell->getCoordinate() << std::endl;
            trapped = true;
        }
    };
    puzzle->forEveryCell(&lam);

    if (!trapped)
    {
        /*
          From the start point of every pipe:
          If the route for that pipe is not complete,
          "flood" all empty cells reachable from it.
          If the other endpoint is not reached, then it is trapped.
          For example, from either side "A" is trapped

              =======================
             | .  X  X  .     A  A (A)
             |
             | . (X)|X  .  .  A (X) .
             |
             | .  .  X  X  .  A  A  .
              ==
             |(A) A  .  X  .  .  .  .
              =======================
         */
        const std::set<PipeId> & ids = puzzle->getPipeIds();
        for (PipeId id : ids)
        {
            std::set<Coordinate> reachable;
            Coordinate start = puzzle->findPipeEnd(id, PipeEnd::PIPE_START);
            reachable.insert(start);
            // Follow existing pipe from start, adding to reachable set
            for (bool follow = true; follow; )
            {
                follow = false;
                for (Direction d : allTraversalDirections)
                {
                    ConstCellPtr p = puzzle->getConstCellAdjacent(start, d);
                    if (p == nullptr)
                        continue;
                    if (p->getPipeId() == id)
                    {
                        if (reachable.find(p->getCoordinate()) != reachable.end())
                            continue;
                        start = p->getCoordinate();
                        reachable.insert(start);
                        follow = true;
                        break;
                    }
                }
            }
            if (start == puzzle->findPipeEnd(id, PipeEnd::PIPE_END) || puzzle->isProxyEnd(id, start))
            {
                // pipe is complete
                //logger << "End found for pipe " << id << " at " << start << std::endl;
                continue;
            }

            // Add reachable empty cells to reachable set.
            //logger << "Recurse reachable for pipe " << id << " from " << start << std::endl;
            if (!recurseReachable(puzzle, start, id, reachable))
            {
#if ANNOUNCE_ENTRAPMENT
                logger << "End not reachable for pipe " << id << " from " << start << std::endl;
#endif
                trapped = true;
                break;
            }
        }
    }
    return trapped;
}

bool detectBadFormation (ConstPuzzlePtr puzzle, const Route & route, PipeId idPipe)
{
    try
    {
        if (adjacencyRuleBroken(puzzle, route))
        {
#if ANNOUNCE_ADJACENCY_LAW_BREAK
            logger << "Adjacency rule broken for " << idPipe << " route " << route << std::endl;
            //logger.stream() << route;
            //logger << std::endl;
            Cell::setOutputConnectorRep(false);
            puzzle->streamPuzzleMatrix(logger.stream());
#endif
            return true;
        }
    }
    catch (const PuzzleException & ex)
    {
        //logger.logException("Puzzle exception in checking adjacency for route", ex);
        logger << "Puzzle exception in checking adjacency for route: " << ex << std::endl;
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
        //logger.logException("Puzzle exception in checking for dead end formation for route ", ex);
        logger << "Puzzle exception in checking for dead end formation for route: " << ex << std::endl;
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
        //logger.logException("Puzzle exception in checking for invalid deviation for route ", ex);
        logger << "Puzzle exception in checking for invalid deviation for route: " << ex << std::endl;
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
