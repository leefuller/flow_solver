#include "../include/Puzzle.h"
#include "formations.h"
#include "Helper.h"
#include "Logger.h"

#include <algorithm>
#include <set>
#include <memory>

static Logger & logger = Logger::getDefaultLogger();

static Direction checkOneAdjacentOnly (std::shared_ptr<const Cell> pCell, Direction d, std::shared_ptr<const Cell> pAdj)
{
    if (pAdj->isEmpty())
        return d;

    // Check if adjacent cell has same pipe id, and is a fixture,
    // which would have an open fixture connection in the direction of the cell at coord.
    if (pAdj->getPipeId() == pCell->getPipeId())
    {
        if (pAdj->getConnection(opposite(d)) == CellConnection::OPEN_FIXTURE)
        {
            // TODO what about checking for other fixtures to endpoint?
            // If the cell is a fixture with 2 other fixtures already connected,
            // then it is should have no remaining traversable directions.
            // FIXME Should not need to check here. The connection algorithms should eliminate that possibility.
            return d;
        }
    }
    return Direction::NONE;
}

static Direction checkForChannel (std::shared_ptr<const Puzzle> puzzle, Coordinate coord, const std::set<Direction> & directions)
{
    /*
     Simplest form is a cell that is a channel by itself.
     Another form occurs where a cell blocks the entry/exit to a channel.

       Example: 'X' blocks entry to the channel.
        ---- . .
           . X .
        ---- . .

      Coordinates left of X and through to leaving the other end of the channel must also be X.
      If the "-" are walls or pipe fixtures, then the cells in the channel and the first out the
      other end are a fixture for the puzzle.
      // TODO also if it is a channel involving any pipe fixture
     */
    for (Direction d : directions)
    {
        std::shared_ptr<const Cell> cellNext = puzzle->getConstCellAdjacent(coord, d);
        if (cellNext == nullptr)
            continue;
        if (!cellNext->isEmpty())
            continue;
        //puzzle->getPlumber()->connect(c1, c2, idPipe, connection);
        if (cellNext->isHorizontalChannel() && (d == Direction::LEFT || d == Direction::RIGHT))
            return d;
        if (cellNext->isVerticalChannel() && (d == Direction::UP || d == Direction::DOWN))
            return d;
    }
    return Direction::NONE;
}

/*
 A formation where 1 or 2 empty cells occur between a pipe and an obstruction,
 where there are 2 fixed connections at the obstruction. ie. A corner
 that can be formed by borders and/or pipe fixtures.

 Fill to the corner
 If up to 2 cells inside a corner are empty, along either wall, then the gap must be filled by the same pipe,
 otherwise it would be a dead end for another pipe, or the adjacency rule would be broken by another pipe.

           G .|         G . .|      Empty cells must be 'G'
           ---          -----
 If 'G' had another border/fixture above, it can be dealt with by the case for channels.
 So, only need to know it is adjacent to at least one wall, and that up to 2 cells
 are empty until reaching the obstruction.
*/
static Direction checkFillToObstruction (std::shared_ptr<const Puzzle> puzzle, std::shared_ptr<const Cell> pCell)
{
#if ANNOUNCE_ONE_WAY_DETECT
    logger << "Try one way algorithm to obstruction" << std::endl;
#endif
    std::set<Direction> dirAdjFixtures; // Directions from cell to immediately adjacent fixtures
    std::shared_ptr<const Cell> pCellNext = pCell;
    // Get adjacent fixtures
    for (Direction dirAdj : allTraversalDirections)
    {
        bool adjBlock = (pCellNext->getBorder(dirAdj) == CellBorder::WALL);
        if (!adjBlock)
        {
            // check if blocked by pipe fixture
            std::shared_ptr<const Cell> pAdj = puzzle->getConstCellAdjacent(pCellNext->getCoordinate(), dirAdj);
            if (pAdj != nullptr)
                adjBlock = pAdj->isFixture();
        }
        if (adjBlock)
            dirAdjFixtures.insert(dirAdj);
    }
    if (dirAdjFixtures.empty())
        return Direction::NONE; // Only useful if cell has a direction blocked

    // Now know that at least one side of the cell has an obstruction.
    // The relative position of the obstruction(s) is in dirAdjFixtures

    // If an adjacent cell, not on the opposite side to the obstruction, is empty,
    // then check whether there is a fixed obstruction on the side of it, that
    // is in the same direction as the adjacent cell is to the current cell.
    // ie. In the illustration below, if X is the current cell (having an obstruction above),
    // then the cells to check for obstructions are left and right.
    // The empty cell right of X is obstructed in the same direction (right) by a wall.
    // (It could also be another fixture from any pipe.)
    // The case for Y, where there are 2 adjacent empty cells before an obstruction is similar.
    // If the first adjacent has an obstruction on the same side that Y has.
    //
    //       =====              =====
    //   . . . X .|       . . . Y . .|
    //

    std::map<Direction, std::vector<std::shared_ptr<const Cell>>> cellsPerDir;
    const Coordinate start = pCell->getCoordinate();
    Coordinate c = start;

    for (Direction dAdjFix : dirAdjFixtures)
    {
        for (Direction d : allTraversalDirections)
        {
            if (d == dAdjFix || d == opposite(dAdjFix))
                continue; // Only interested in blockages adjacent to traversal direction

            unsigned distance = puzzle->gapToObstruction(start, d);
            if (!distance || distance > 2)
                continue; // only interested in gaps of 1 or 2

            pCellNext = puzzle->getConstCellAdjacent(c, d);
            // Cells until obstruction need an obstruction aligned with the obstruction for the prior cell
            if (pCellNext->isBorderOpen(dAdjFix)) // TODO check for pipe fixture in the direction
                continue;

            if (distance == 1) // pCellNext is next to the obstruction. This is the first case in comments above.
            {
                if (!pCellNext->isBorderOpen(d))
                    return d;
                // else obstruction is the next cell in the same direction, which needs to be a pipe fixture.
                pCellNext = puzzle->getConstCellAdjacent(pCellNext->getCoordinate(), d);
                if (pCellNext == nullptr || pCellNext->isFixture())
                    return d;
                continue; // try another direction
            }
            // else distance is 2.
            // We know the next cell in the traversal direction must be empty
            // So check the one next to it in the same direction.
            coordinateChange(c, d, 2);
            pCellNext = puzzle->getConstCellAtCoordinate(c);
            // The cell needs an obstruction aligned with the obstruction for the prior cell,
            // as well as the obstruction in traversal direction
            if (pCellNext->isBorderOpen(dAdjFix))
                continue; // TODO check for pipe fixture in the direction

            if (!pCellNext->isBorderOpen(d))
                return d;
            // else obstruction is the next cell in the same direction, which needs to be a pipe fixture.
            pCellNext = puzzle->getConstCellAdjacent(pCellNext->getCoordinate(), d);
            if (pCellNext == nullptr || pCellNext->isFixture())
                return d;
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
Direction theOnlyWay (std::shared_ptr<const Puzzle> puzzle, Coordinate coord)
{
#if ANNOUNCE_ONE_WAY_DETECT
    logger << "Check only way formations." << std::endl;
#endif
    std::shared_ptr<const Cell> pCell = puzzle->getConstCellAtCoordinate(coord);
    if (pCell->isEmpty())
    {
#if ANNOUNCE_ONE_WAY_DETECT
        logger << coord << " is empty" << std::endl;
#endif
        return Direction::NONE;
    }
    PipeId idPipe = pCell->getPipeId();
    std::map<Direction, std::shared_ptr<const Cell>> adjacentCells = puzzle->getAdjacentCells(pCell);

    for (auto it = adjacentCells.begin(); it != adjacentCells.end();) {
        if (it->second == nullptr)
            it = adjacentCells.erase(it);
        else
            ++it;
    }
    // adjacentCells now only contains existing cells.
    if (adjacentCells.size() == 1)
    {
        Direction d = checkOneAdjacentOnly(pCell, std::begin(adjacentCells)->first, std::begin(adjacentCells)->second);
        if (d != Direction::NONE)
            return d;
    }

#if ANNOUNCE_ONE_WAY_DETECT
    logger << coord << " has " << adjacentCells.size() << " adjacent cells" << std::endl;
#endif

    std::set<Direction> traversableDirections = Helper::getNowTraversableDirections(puzzle, coord, idPipe);
#if ANNOUNCE_ONE_WAY_DETECT
    logger << coord << " has " << traversableDirections.size() << " traversable direction(s)" << std::endl;
#endif
    if (traversableDirections.size() == 0)
        return Direction::NONE;
    if (traversableDirections.size() == 1)
    {
#if ANNOUNCE_ONE_WAY_DETECT
        logger << "traversable " << asString(*(traversableDirections.begin())) << std::endl;
#endif
        return *(traversableDirections.begin());
    }

    Direction dc = checkForChannel(puzzle, coord, traversableDirections);
    if (dc != Direction::NONE)
        return dc;

    Direction d = checkFillToObstruction(puzzle, pCell);
    if (d != Direction::NONE)
        return d;

    return Direction::NONE;
}
