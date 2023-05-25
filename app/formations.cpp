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
 * @param puzzle    The puzzle state
 * @param coord     Coordinate for cell
 * @param idPipe    Pipe identifier
 * @param inwardDirection   The side from which the cell is entered.
 * @return true if a deviation has formed that would invalidate any route possible there
 */
bool detectInvalidDeviation (std::shared_ptr<const Puzzle> puzzle, const Route & route, PipeId idPipe)
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
 * Entrapment exists where a route traps empty cells,
 * or cells that have one endpoint for a particular pipe, but not both,
 * within an area that prevents any other pipe from reaching.
 * It is only worthwhile calling this function once the route is complete.
 * @param puzzle    The puzzle state being assessed
 * @param route     The route being considered
 * @param idPipe    The pipe id for the route
 * @return true if the route traps something
 */
bool detectEntrapment (std::shared_ptr<const Puzzle> puzzle, const Route & route, PipeId idPipe)
{
    // Note, there can be more than 1 entrapped area. Return true on detection of first.
    // An area entrapped could be a lot of the puzzle.
    // An endpoint is trapped if it cannot reach the corresponding endpoint, where the route acts as a wall.

    // TODO

    return false;
}

bool detectBadFormation (std::shared_ptr<const Puzzle> puzzle, const Route & route, PipeId idPipe)
{
    try
    {
        if (adjacencyRuleBroken(puzzle, route))
            return true;
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
            return true;
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
            return true;
    }
    catch (const PuzzleException & ex)
    {
        //logger.logException("Puzzle exception in checking for invalid deviation for route ", ex);
        logger << "Puzzle exception in checking for invalid deviation for route: " << ex << std::endl;
        throw;
    }
    if (detectEntrapment(puzzle, route, idPipe))
        return true;

    return false;
}
