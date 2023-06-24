#include "../include/Puzzle.h"
#include "formations.h"
#include "Logger.h"

static Logger & logger = Logger::getDefaultLogger();

/**
 * Check whether given cell is a dead end, derived from what is immediately adjacent.
 *
 * @param puzzle    The puzzle state being assessed
 * @param coord     The coordinate being assessed
 * @return true if dead end formation exists
 */
bool detectDeadEndFormation (ConstPuzzlePtr puzzle, Coordinate coord)
{
    std::set<Direction> directions = puzzle->getConnectedDirections(coord);
    if (directions.size() == 0)
        return true;
    ConstCellPtr pCell = puzzle->getConstCellAtCoordinate(coord);
    if (pCell == nullptr) // should not happen
        return false;
    if (pCell->isEndpoint()) // An endpoint is never a dead end
        return false;

    PipeId idPipe = pCell->getPipeId();
    if (pCell->countWalls() > 2)
        return true;
    unsigned matchAdjacent = 0;

    //std::set<Direction> traversableDirections = Helper::getNowTraversableDirections(puzzle, coord, idPipe);
    std::map<Direction, ConstCellPtr> adjacent = puzzle->getAdjacentCellsInTraversalDirections(coord, true);

    unsigned emptyAdjacent = std::count_if(adjacent.begin(), adjacent.end(),
            [](auto it){ return it.second != nullptr && it.second->getPipeId() == NO_PIPE_ID; });
    if (emptyAdjacent > 1)
    {
        // Can be no immediate dead end with more than 1 empty, unobstructed neighbour
        return false;
    }

    if (idPipe == NO_PIPE_ID)
    {
        /* OK if there is an empty cell adjacent

            A          A
          B . .       |. .
            C          B
         */
        if (emptyAdjacent > 0)
            return false;

        /* OK if 2 adjacent cells have the same pipe

             A        A
           B . C     |.|
             C        A

          But not more than 2, because there would be 3 connections, which is invalid,
          or dead end for another pipe:
             A
           C . C
             C
         */
        for (Direction d : allTraversalDirections)
        {
            if (adjacent[d] == nullptr)
                continue;
            PipeId id = adjacent[d]->getPipeId();
            // Check for adjacent matches
            unsigned count = 0;
            for (Direction d2 : allTraversalDirections)
            {
                if (adjacent[d2] == nullptr)
                    continue;
                if (adjacent[d2]->getPipeId() == id)
                    ++count;
            }
#if ANNOUNCE_DEAD_END_DETECT
            logger << "Matches next to empty cell at " << coord << " = " << count << std::endl;
#endif
            if (count == 2)
                return false;
        }
        /* Dead end:

             A        A
           B . D     |.|
             C        B
        */
    }
    else // cell not empty
    {
        /* OK if center matches an adjacent pipe, and is either an endpoint,
          or has an adjacent empty cell. Endpoint represented by "(X)"

             A         A        A
           B X X     B(X).    B(X)X
             .         C        C
         */
        // Count number of cells adjacent matching the pipe.
        matchAdjacent = std::count_if(adjacent.begin(), adjacent.end(),
                [idPipe](auto it){ return it.second != nullptr && it.second->getPipeId() == idPipe; });
        if ((emptyAdjacent + matchAdjacent) > 1)
            return false;
        // Ok for only 1 match or 1 empty adjacent, if an endpoint
        if ((matchAdjacent == 1 || emptyAdjacent == 1) && pCell->isEndpoint())
            return false;
    }
#if ANNOUNCE_DEAD_END_DETECT
    logger << "Dead end at " << coord << " for " << (idPipe == NO_PIPE_ID ? "empty cell" : std::to_string(idPipe));
    logger << ": Adjacent empty: " << emptyAdjacent << ", match " << matchAdjacent << std::endl;
#endif
    return true;
}

/**
 * Detect a dead end formation that may be caused by a given route.
 *
 * @param puzzle    The puzzle state being assessed
 * @param route     The route being assessed
 * @param idPipe    The pipe id for a route, or NO_PIPE_ID if only considering walls.
 * @return true if dead end formation exists
 */
bool detectDeadEndFormation (ConstPuzzlePtr puzzle, const Route & route, PipeId idPipe)
{
    /*
     Where "|" and "-" is wall, or another pipe, and "[X]" is a pipe start or end point:

    Case 1: A single cell dead end exists for an empty cell when:
     |.|  and similarly for the 3 other directions.
     ---
         Only 1 cell needs checking in each direction adjacent to each cell in the route.
     */
    for (Coordinate coord : route)
    {
        // Check cells adjacent to route
        std::set<Direction> directions = puzzle->getConnectedDirections(coord);
        for (Direction direction : directions)
        {
            Coordinate coordCheck = coord;
            if (!coordinateChange(coordCheck, direction))
                continue;
            if (coordinateInRoute(coordCheck, route))
                continue;
            if (detectDeadEndFormation(puzzle, coordCheck))
            {
#if ANNOUNCE_DEAD_END_DETECT
                logger << "Dead end assessing " << coordCheck << " for route " << route << std::endl;
#endif
                return true;
            }
        }
    }

    /*
    Case 2: TODO handle dead in 3 cell 'U' turn?
     Invalid formation that would otherwise be detected via another dead end rule,
     or the adjacency rule, if not handled here.
     With X as a start/end point, these are invalid:

       . . .        |. . .|     |. . . .|        .|. .
      |. X .|       |X . .|     |. X . .|       |. X .|
       -----         -----       -------         -----


    Valid:
       .|. .
      |. X . .
       -----

     */

    return false;
}
