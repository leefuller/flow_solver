#include "Puzzle.h"
#include "Cell.h"
#include "PuzzleException.h"
#include "formations.h"
#include "Logger.h"

#include <algorithm>

static Logger & logger = Logger::getDefaultLogger();

/**
 * Find a pipe endpoint that may be reachable via empty cells only.
 * @param puzzle    Puzzle being assessed
 * @param c         Start coordinate
 * @param idPipe    Identifier of pipe
 * @param visited   Coordinates visited so far
 * @return true if the endpoint is found.
 */
static bool recurseReachable (ConstPuzzlePtr puzzle, Coordinate c, PipeId idPipe, std::set<Coordinate> & visited)
{
    for (Direction d : allTraversalDirections)
    {
        if (!puzzle->isCoordinateChangeValid (c, d)) // disregarding walls and pipes
            continue;

        ConstCellPtr pAdj = puzzle->getConstCellAdjacent(c, d);
        if (pAdj == nullptr)
            continue;
        if (std::find(std::begin(visited), std::end(visited), pAdj->getCoordinate()) != visited.end()) // already visited
            continue;
        if (!pAdj->isEmpty())
        {
            // FIXME: This should be divided so that same pipe not end will be followed??
            if (pAdj->getPipeId() == idPipe && (pAdj->getEndpoint() == PipeEnd::PIPE_END || puzzle->isProxyEnd(idPipe, pAdj->getCoordinate())))
            {
                // found
#if (ANNOUNCE_SOLVER_DETAIL) && (ANNOUNCE_ENTRAPMENT)
                logger << "Reachable end found for pipe " << idPipe << " at " << pAdj->getCoordinate() << std::endl;
#endif
                return true;
            }
#if (ANNOUNCE_SOLVER_DETAIL) && (ANNOUNCE_ENTRAPMENT)
            if (interestingPipe(idPipe))
                logger << "Keep searching directions from " << idPipe << " " << c << std::endl;
#endif
            continue;
        }
        visited.insert(c);
        if (recurseReachable(puzzle, pAdj->getCoordinate(), idPipe, visited))
            return true;
    }
    return false;
}

/**
 * Detect whether a given route traps anything.
 * 
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

    if (route.empty())
        return false;

    bool trapped = false;

    // The simplest case is where a single empty cell is trapped.
    // (ie. on each side is a border or other pipe.)
    std::function<void(ConstCellPtr)> lam = [&puzzle, &trapped, idPipe](ConstCellPtr cell){
        if (!puzzle->isCellReachable(cell->getCoordinate()))
            return;
        // Dead end formation can indicate a cell is trapped
        if (detectDeadEndFormation(puzzle, cell->getCoordinate()))
        {
#if ANNOUNCE_ENTRAPMENT
            logger << "Trap at dead end " << cell->getCoordinate() << std::endl;
#endif
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
            std::set<Coordinate> followed;
            Coordinate start = puzzle->findPipeEnd(id, PipeEnd::PIPE_START);
            followed.insert(start);
            // Follow existing pipe from start, adding to followed set
            for (bool follow = true; follow; )
            {
                follow = false;
                for (Direction d : allTraversalDirections)
                {
                    ConstCellPtr p = puzzle->getConstCellAdjacent(start, d); // disregards inner walls
                    if (p == nullptr)
                        continue;
                    if (p->getBorder(opposite(d)) == CellBorder::WALL)
                        continue;
                    if (p->getPipeId() == id)
                    {
                        if (followed.find(p->getCoordinate()) != followed.end())
                            continue;
                        start = p->getCoordinate();
                        followed.insert(start);
                        follow = true;
                        break;
                    }
                }
            }
            if (start == puzzle->findPipeEnd(id, PipeEnd::PIPE_END) || puzzle->isProxyEnd(id, start))
            {
                // pipe is complete
#if (ANNOUNCE_SOLVER_DETAIL) && (ANNOUNCE_ENTRAPMENT)
                logger << "End found for pipe " << id << " at " << start << "(in detectEntrapment)" << std::endl;
#endif
                continue;
            }

            // Add reachable empty cells to reachable set.
#if (ANNOUNCE_SOLVER_DETAIL) && (ANNOUNCE_ENTRAPMENT)
            logger << "Recurse reachable for pipe " << id << " from " << start << std::endl;
            /*logger << "Followed ";
            std::for_each(std::begin(followed), std::end(followed), [](auto it){ logger << it << ' '; });
            logger << std::endl;*/
#endif
            if (!recurseReachable(puzzle, start, id, followed))
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
