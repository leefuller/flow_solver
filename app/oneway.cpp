#include "../include/Puzzle.h"
#include "formations.h"
#include "Helper.h"
#include "Logger.h"

#include <algorithm>
#include <set>
#include <memory>

static Logger & logger = Logger::getDefaultLogger();

static Direction checkForChannel (ConstPuzzlePtr puzzle, Coordinate coord, const std::set<Direction> & directions)
{
    /*
     Simplest form is a cell that is a channel by itself.
     Another form occurs where a cell blocks the entry/exit to a channel.

       Example: 'X' blocks entry to the channel.
        ---- . .
           . X .
        ---- . .

      Coordinates left of X and through to leaving the other end of the channel must also be X.
     */
    for (Direction d : directions)
    {
        ConstCellPtr cellNext = puzzle->getConstCellAdjacent(coord, d);
        if (cellNext == nullptr)
            continue;
        if (!cellNext->isEmpty())
            continue;
        if (cellNext->isHorizontalChannel() && (d == Direction::WEST || d == Direction::EAST))
            return d;
        if (cellNext->isVerticalChannel() && (d == Direction::NORTH || d == Direction::SOUTH))
            return d;
    }
    return Direction::NONE;
}

/*
 A formation where 1 or 2 empty cells occur between a pipe and an obstruction,
 along a line adjacent to a fixture.
 The obstruction can only be a wall, or a fixture for the same pipe.

 Case 1: One empty cell:
     If X is a fixture, then the adjacent cell must be X,
     otherwise it would be a dead end for another pipe.

           X .|    or   X . X
           ---          ---

 Case 2: Two empty cells:

 If 2 cells along a wall to an obstruction are empty, then the gap must be filled by the same pipe,
 unless the adjacency law would not be broken.

             . .
           X . .|  or   X . . X
           -----        -----
 If 'X' had another border/fixture above, it can be dealt with by the case for channels.
 So, only need to know it is adjacent to at least one wall, and that up to 2 cells
 along the wall are empty until reaching the obstruction.

 If the adjacency law cannot be broken in the corner, then it is not valid to fill to the corner.
 For example, in this formation, the adjacency law cannot be broken in the corner,
 so the cell right from 'X' might not be 'X'

             .|.
           X . .|
           -----
*/
Direction checkFillToCorner (ConstPuzzlePtr puzzle, ConstCellPtr pCell)
{
    if (pCell == nullptr)
        return Direction::NONE;
    std::set<Direction> dirAdjFixtures; // Directions from cell to immediately adjacent fixtures
    // Get adjacent fixtures
    for (Direction dirAdj : allTraversalDirections)
    {
        if (pCell->getBorder(dirAdj) == CellBorder::WALL)
            dirAdjFixtures.insert(dirAdj);
    }
    if (dirAdjFixtures.empty())
        return Direction::NONE; // Only useful if cell has a direction blocked

    // Now know that at least one side of the cell has an obstruction.
    // The relative position of the obstruction(s) is in dirAdjFixtures

    // If an adjacent cell, not on the opposite side to the obstruction, is empty,
    // then check whether there is a wall on the side of it that
    // is in the same direction as the adjacent wall is to the current cell.

    const Coordinate start = pCell->getCoordinate();
    Coordinate c = start;

    for (Direction dAdjFix : dirAdjFixtures) // for each wall adjacent (to 'X')
    {
        for (Direction d : allTraversalDirections)
        {
            if (d == dAdjFix || d == opposite(dAdjFix))
                continue; // Only interested in blockages adjacent to traversal direction

            // Direction d is parallel to the adjacent wall.

            unsigned distance = puzzle->gapToObstruction(start, d);
            if (!distance || distance > 2)
                continue; // only interested in gaps of 1 or 2

            ConstCellPtr pCellNext = puzzle->getConstCellAdjacent(c, d); // cell first step in direction towards obstruction
            if (pCellNext->isBorderOpen(dAdjFix)) // TODO check for pipe fixture in the direction
                continue; // no aligned obstruction

#if ANNOUNCE_ONE_WAY_DETECT
            //pCellObstruct->describe(logger.stream()); logger << std::endl;
            logger << "Distance from " << start << " to obstruction " << asString(d) << " is " << distance << std::endl;
#endif
            ConstCellPtr pCellBeforeObstruct = nullptr;

            // At this point, pCellNext (next to pCell) is the first cell towards the obstruction.

            if (distance == 1)
                pCellBeforeObstruct = pCellNext;
            else // else distance is 2
                pCellBeforeObstruct = puzzle->getConstCellAdjacent(pCellNext->getCoordinate(), d);

            // pCellBeforeObstruct is immediately prior to the obstruction.
            // The obstruction could be it's own wall, or the adjacent cell.

            if (pCellBeforeObstruct->isBorderOpen(d))
            {
                // The obstruction is the adjacent cell.
                ConstCellPtr p = puzzle->getConstCellAdjacent(pCellBeforeObstruct->getCoordinate(), d);
                // If it is a different pipe, then bail out.
                if (p->getPipeId() != pCell->getPipeId())
                    continue;
            }

            // At this point, it has been determined that the obstruction is a wall or compatible pipe.

            if (distance == 1)
                return d; // The gap can be filled

            // distance is 2.
            if (pCellBeforeObstruct->isBorderOpen(dAdjFix)) // TODO check for pipe fixture in the direction
                continue; // no aligned obstruction

            // Check if the adjacency law could be broken in the corner.
            // If so, then can fill to the corner.
            // In the diagram below, the adjacency law could be broken by the cell marked 'o',
            // regardless of it being empty or containing a pipe. So X can go to corner.
            // If it contained a pipe, then X is effectively at the head of a channel.
            // So, leaving channel logic to deal with 'o' being a pipe, if the adjacency law
            // would otherwise be broken in the corner, then the corner should contain 'X'
            //
            //     -----
            //     X . .|
            //       o .
            //
            // The only thing that can change the corner from having to be X is the
            // existence of a wall dividing the adjacent pair, as in:
            //     -----
            //     X . .|   Now X does not have to fill the gap, because 'o' might go there.
            //       o|.

            // Either of 2 inner walls in the corner 2x2 would cause a dead end.
            // Another means X must go to the corner in a channel.
            // Only 1 makes a valid formation with X not forced to the corner: the one dividing the adjacent pair.

            // The adjacent pair is adjacent to those following the wall to the corner.
            /*
            pCell is the 'X'
            dAdjFix is the side of 'X' for the wall that tracks to the corner.
            pCellNext and pCellCorner are the cells between 'x' and the corner,
            then the pair of interest is on the side opposite(dAdjFix) of pCellNext and pCellCorner
            */
            std::pair<ConstCellPtr, ConstCellPtr> pPair = std::make_pair<ConstCellPtr, ConstCellPtr>(
                    puzzle->getConstCellAdjacent(pCellNext->getCoordinate(), opposite(dAdjFix)),
                    puzzle->getConstCellAdjacent(pCellBeforeObstruct->getCoordinate(), opposite(dAdjFix)));
            if (pPair.first != nullptr && pPair.second != nullptr)
            {
                // Check if pair is divided by wall
                if (pPair.first->isBorderOpen(d))
                    return d; // Not divided so 'X' must go towards corner
            }
        }
    }
    return Direction::NONE;
}

/**
 * Detect a formation where there is only one direction to move from a coordinate,
 * either because there is only one available direction, or
 * because another rule would invalidate all alternative directions.
 * @param puzzle    The puzzle state being assessed
 * @param coord     Coordinate that is the point from which we might only go one way
 * @return direction that is the only valid way from the coordinate (or Direction::NONE if none, or more that 1)
 */
Direction theOnlyWay (ConstPuzzlePtr puzzle, Coordinate coord)
{
#if ANNOUNCE_ONE_WAY_DETECT
    //logger << "Check only way formations." << std::endl;
#endif
    ConstCellPtr pCell = puzzle->getConstCellAtCoordinate(coord);
    if (pCell->isEmpty())
    {
#if ANNOUNCE_ONE_WAY_DETECT
        //logger << coord << " is empty" << std::endl;
#endif
        return Direction::NONE;
    }
    PipeId idPipe = pCell->getPipeId();
    std::map<Direction, ConstCellPtr> cellGroup = puzzle->getSurroundingCells(coord);

    for (auto it = cellGroup.begin(); it != cellGroup.end();) {
        if (it->second == nullptr)
            it = cellGroup.erase(it);
        else if (isDiagonal(it->first))
            it = cellGroup.erase(it); // no diagonal traversal
        else
            ++it;
    }
    // adjacentCells now only contains existing cells.
    if (cellGroup.size() == 1) // Only 1 cell adjacent
    {
        Direction d = std::begin(cellGroup)->first;
        ConstCellPtr pAdj = std::begin(cellGroup)->second;
        if (pAdj->isEmpty())
            return d;
        if (pAdj->getPipeId() == pCell->getPipeId())
        {
            // Connect to adjacent cell with same pipe
            if (pAdj->getConnection(opposite(d)) == CellConnection::OPEN_FIXTURE)
            {
#if ANNOUNCE_ONE_WAY_DETECT
                logger << pCell->getCoordinate() << " " << asString(d) << " can connect to open fixture at " << pAdj->getCoordinate() << std::endl;
#endif
                return d;
            }
            else
            {
#if ANNOUNCE_ONE_WAY_DETECT
                logger << pCell->getCoordinate() << " " << asString(d) << " has connector " << pAdj->getConnection(opposite(d)) << std::endl;
#endif
            }
        }
        return Direction::NONE;
    }

#if ANNOUNCE_ONE_WAY_DETECT
    //logger << coord << " has " << cellGroup.size() << " adjacent cells" << std::endl;
#endif

    std::set<Direction> traversableDirections = Helper::getNowTraversableDirections(puzzle, coord, idPipe);
#if ANNOUNCE_ONE_WAY_DETECT
    //logger << coord << " has " << traversableDirections.size() << " traversable direction(s)" << std::endl;
#endif
    if (traversableDirections.size() == 0)
        return Direction::NONE;
    if (traversableDirections.size() == 1)
    {
#if ANNOUNCE_ONE_WAY_DETECT
        //logger << "traversable " << asString(*(traversableDirections.begin())) << std::endl;
#endif
        return *(traversableDirections.begin());
    }
    for (auto it = traversableDirections.begin(); it != traversableDirections.end(); ++it)
    {
        ConstCellPtr pCellNext = puzzle->getConstCellAdjacent(pCell->getCoordinate(), *it);
        if (pCellNext->isEndpoint())
            return *it;
    }

    Direction dc = checkForChannel(puzzle, coord, traversableDirections);
    if (dc != Direction::NONE)
        return dc;
    return Direction::NONE;
}
