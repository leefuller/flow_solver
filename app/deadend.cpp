#include "../include/Puzzle.h"
#include "formations.h"
#include "Helper.h"

/**
 * Check whether given cell is a dead end, derived from what is immediately adjacent.
 *
 * @param puzzle    The puzzle state being assessed
 * @param coord     The coordinate being assessed
 * @param idPipe    The pipe id for a route, or NO_PIPE_ID if only considering walls.
 * @return true if dead end formation exists
 */
static bool detectDeadEndFormation (ConstPuzzlePtr puzzle, Coordinate coord)
{
    std::set<Direction> directions = puzzle->getConnectedDirections(coord);
    if (directions.size() == 0)
        return true;
    ConstCellPtr pCell = puzzle->getConstCellAtCoordinate(coord);
    if (pCell == nullptr) // should not happen
        return false;

    // TODO improve logic here
    std::set<Direction> wallsAdjacentDirections;
    std::set<Direction> pipesAdjacentDirections;
    if (Helper::getObstructedDirections(puzzle, pCell, wallsAdjacentDirections, pipesAdjacentDirections) < 3)
        return false;

    if (wallsAdjacentDirections.size() == 3 && !pCell->isEndpoint())
        return true;

    unsigned countWalls = wallsAdjacentDirections.size();
    unsigned emptyAdjacent = 4 - countWalls - pipesAdjacentDirections.size();

    PipeId idPipe = pCell->getPipeId();

    // Create list of adjacent pipes, and set of the adjacent pipe ids
    std::vector<ConstCellPtr> adjacentPipes;
    std::set<PipeId> adjacentIds;
    for (Direction d : allTraversalDirections)
    {
        auto matchDirection = [d](auto it){ return d == it; };
        if (std::find_if(pipesAdjacentDirections.begin(), pipesAdjacentDirections.end(), matchDirection)
                != pipesAdjacentDirections.end()) // direction d has an adjacent pipe
        {
            ConstCellPtr pCellAdjacent = puzzle->getConstCellAdjacent(coord, d);
            adjacentIds.insert(pCellAdjacent->getPipeId());
            adjacentPipes.push_back(pCellAdjacent);
        }
    }

    /* OK: If center is empty:
             Any empty adjacent reachable, or no more than 2 adjacent cells as blockers, unless 2 are the same
           If center is not empty:
             2 empty adjacent, or 1 empty adjacent and 1 matching center adjacent

            A       A        A       A       A       A        N
          B . .   B . C    B X .   B X X   .|.|.    |. .    P . N
            C       C        .       .       A       B        P

       BAD:
            A       A        A       A       A
          B . D   B A D    B X .   B X .   .|.|.
            C       C        C       B       B
     */

    if (idPipe == NO_PIPE_ID)
    {
        if (emptyAdjacent && (countWalls < 2))
            return false;
#if ANNOUNCE_DEAD_END_DETECT
        if (countWalls + adjacentIds.size() > 3)
        {
            logger << "Dead end at " << coord;//<< std::endl;
            logger << ": " << countWalls << " walls; Adjacent pipes: " << adjacentPipes.size() << std::endl;
        }
#endif
        return (countWalls + adjacentIds.size()) > 3;
    }
    // Cell contains a pipe
    // Count number of cells adjacent matching the pipe.
    unsigned matchAdjacent = std::count_if(adjacentPipes.begin(), adjacentPipes.end(), [idPipe](ConstCellPtr p){ return p->getPipeId() == idPipe; });
    //unsigned matchAdjacent = matchAdjacentPipes.size();
    if ((emptyAdjacent + matchAdjacent) > 1)
        return false;
    // Ok for only 1 match or 1 empty adjacent, if an endpoint
    if ((matchAdjacent == 1 || emptyAdjacent == 1) && pCell->isEndpoint())
        return false;
#if ANNOUNCE_DEAD_END_DETECT
    logger << "Dead end at " << coord << " for pipe " << idPipe ;//<< std::endl;
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
     Case 2: 2 adjacent cells with 3 walls/pipes, without a wall to separate the adjacent pair

    Dead end            Valid, unless 1 corner has an endpoint, which would form a deadend case 1
      . .                 .|.
     |. .|               |. .|
      ---                 ---

      For the dead end case, another wall for either cell doesn't matter, because it would form a
      single cell dead end, detected in case 1.

      The content of either cell does not matter, even if there is an endpoint, because:
      - If the 2 cells are occupied by the same pipe, the adjacency rule will be broken.
      - If there are 2 separate pipes, or 1 + empty, then there is a single cell dead end for both.

      The formation can be detected if the number of obstructions for each adjacent cell is 2,
      and 2 borders align, and 2 borders oppose.
     */
    for (Coordinate coord : route)
    {
        // Get 2 adjacent cells that are not part of the route
        std::set<Direction> directions = puzzle->getConnectedDirections(coord);
        for (Direction direction : directions)
        {
            Coordinate coordCheck = coord;
            if (!coordinateChange(coordCheck, direction))
                continue;
            if (coordinateInRoute(coordCheck, route))
                continue;

            ConstCellPtr pCell = puzzle->getConstCellAtCoordinate(coordCheck);
            for (Direction directionAdjacent : puzzle->getConnectedDirections(coordCheck))
            {
                ConstCellPtr pCellAdjacent = puzzle->getConstCellAdjacent(coordCheck, directionAdjacent);
                if (pCellAdjacent == nullptr)
                    continue;
                if (coordinateInRoute(pCellAdjacent->getCoordinate(), route))
                    continue;
                // Now have 2 adjacent cells that are not in the route

                std::set<Direction> wallsAdjacentDirections1;
                std::set<Direction> pipesAdjacentDirections1;
                if (Helper::getObstructedDirections(puzzle, pCell, wallsAdjacentDirections1, pipesAdjacentDirections1) != 2)
                    continue;
                std::set<Direction> wallsAdjacentDirections2;
                std::set<Direction> pipesAdjacentDirections2;
                if (Helper::getObstructedDirections(puzzle, pCellAdjacent, wallsAdjacentDirections2, pipesAdjacentDirections2) != 2)
                    continue;
                // The 2 adjacent cells each have 2 obstructions

                bool alignedLongBorder = false;
                bool opposingShortBorders = false;
                if (isVertical(directionAdjacent))
                {
                    alignedLongBorder =
                         (pCell->getBorder(Direction::LEFT) == pCellAdjacent->getBorder(Direction::LEFT) ||
                          pCell->getBorder(Direction::RIGHT) == pCellAdjacent->getBorder(Direction::RIGHT));
                    if (directionAdjacent == Direction::UP) // pCellAdjacent is above pCell
                    {
                        opposingShortBorders =
                                pCellAdjacent->getBorder(Direction::UP) == CellBorder::WALL &&
                                pCell->getBorder(Direction::DOWN) == CellBorder::WALL;
                    }
                    else // pCellAdjacent is below pCell
                    {
                        opposingShortBorders =
                                pCell->getBorder(Direction::UP) == CellBorder::WALL &&
                                pCellAdjacent->getBorder(Direction::DOWN) == CellBorder::WALL;
                    }
                }
                else // horizontal
                {
                    alignedLongBorder =
                         (pCell->getBorder(Direction::UP) == pCellAdjacent->getBorder(Direction::UP) ||
                          pCell->getBorder(Direction::DOWN) == pCellAdjacent->getBorder(Direction::DOWN));
                    if (directionAdjacent == Direction::LEFT) // pCellAdjacent is left of pCell
                    {
                        opposingShortBorders =
                                pCellAdjacent->getBorder(Direction::LEFT) == CellBorder::WALL &&
                                pCell->getBorder(Direction::RIGHT) == CellBorder::WALL;
                    }
                    else // pCellAdjacent is right of pCell
                    {
                        opposingShortBorders =
                                pCell->getBorder(Direction::LEFT) == CellBorder::WALL &&
                                pCellAdjacent->getBorder(Direction::RIGHT) == CellBorder::WALL;
                    }
                }
                if (alignedLongBorder && opposingShortBorders)
                {
                    /* The pair is a dead end if the adjacent reachable pair is not divided by a wall

                    Valid:      .|.
                               |. .|
                                ---
                     */
                    // Get one of the adjacent pair, which is on the opposite side of the long border.
                    if (isVertical(directionAdjacent))
                    {
                        // The adjacent pair is left or right, depending upon the long border location
                        if (pCell->getBorder(Direction::LEFT) == CellBorder::WALL)
                            pCellAdjacent = puzzle->getConstCellAdjacent(pCell->getCoordinate(), Direction::RIGHT);
                        else // The adjacent direction is LEFT
                            pCellAdjacent = puzzle->getConstCellAdjacent(pCell->getCoordinate(), Direction::LEFT);
                    }
                    else // horizontal
                    {
                        // The adjacent pair is above or below, depending upon the long border location
                        if (pCell->getBorder(Direction::UP) == CellBorder::WALL)
                            pCellAdjacent = puzzle->getConstCellAdjacent(pCell->getCoordinate(), Direction::DOWN);
                        else
                            pCellAdjacent = puzzle->getConstCellAdjacent(pCell->getCoordinate(), Direction::UP);
                    }
                    if (pCellAdjacent->getBorder(directionAdjacent) != CellBorder::WALL)
                    {
#if ANNOUNCE_DEAD_END_DETECT
                        logger << "Dead end assessing adjacent pair at " << coordCheck << " for route " << route << std::endl;
#endif
                        return true;
                    }
                }
            }
        } // for direction
    } // for coordinate in route


    /*
    Case 3: TODO?
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
