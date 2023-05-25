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

    /* TODO
    Direction d = checkFillToObstruction(puzzle, pCell);
    if (d != Direction::NONE)
        return d;*/

    return Direction::NONE;
}
