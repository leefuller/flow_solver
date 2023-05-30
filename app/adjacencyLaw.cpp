#include "formations.h"
#include "../include/Puzzle.h"
#include "../include/PuzzleException.h"
#include "Logger.h"

static Logger & logger = Logger::getDefaultLogger();

/*
 * The adjacency law says that cells in a 2x2 formation without a dividing wall
 * cannot have the same pipe.
 *
 * This is not a rule of the puzzle, but no solution has such a formation.
 *
 * The adjacency rule is intended to eliminate a route containing a formation like these:
 * X X X       X X X
 *   X X         X|X
 * (The formations are invalid because the route could just be X X X. There is no need for diversion.)
 *
 * A formation with separating wall like shown here is valid:
 *          X|X
 *          X X
 */

/**
 * Check whether quadrant of cells breaks the adjacency law.
 * @param quad  Array of cells in the order given by central, then clockwise.
 * @param dQuad Direction of quadrant from center of a 3x3 group. ie. NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST
 * @return true if the adjacency law is broken
 */
bool isAdjacencyLawBroken (const std::array<ConstCellPtr, 4> & quad, Direction dQuad)
{
    // The law can only be broken if all 3 cells adjacent to the central one are empty or have the same pipe,
    // without any intervening wall.
    PipeId id = NO_PIPE_ID;
    std::array<PipeId, 4> pipes = { NO_PIPE_ID, NO_PIPE_ID, NO_PIPE_ID, NO_PIPE_ID };
    for (ConstCellPtr p : quad)
    {
        if (p == nullptr) // no cell at location
            return false;
        if (p->getPipeId() == NO_PIPE_ID) // empty cell
            return false;
        if (id == NO_PIPE_ID) // first not empty
            id = p->getPipeId();
        else if (id != p->getPipeId()) // different pipe
            return false;
    }
    // All cells adjacent to central contain same pipe

    // Any inner wall means the adjacency law cannot be broken.
    // The clockwise ordering in the quad array means:
    // The 2nd and 4th cells in the array share a border with the center cell.
    // The 3rd cell in the array is the one in the quadrant direction.

    // So an inner wall can be detected by looking at the center and diagonal cells only

    switch (dQuad)
    {
        case Direction::NORTH_EAST:
            /*
               .XX      1 2
               .XX      0 3
               ...
             */
            if (!quad[0]->isBorderOpen(Direction::NORTH) ||
                !quad[0]->isBorderOpen(Direction::EAST))
                return false;
            if (!quad[2]->isBorderOpen(Direction::WEST) ||
                !quad[2]->isBorderOpen(Direction::SOUTH))
                return false;
            break;
        case Direction::SOUTH_EAST:
            /*
               ...
               .XX      0 1
               .XX      3 2
             */
            if (!quad[0]->isBorderOpen(Direction::EAST) ||
                !quad[0]->isBorderOpen(Direction::SOUTH))
                return false;
            if (!quad[2]->isBorderOpen(Direction::NORTH) ||
                !quad[2]->isBorderOpen(Direction::WEST))
                return false;
            break;
        case Direction::SOUTH_WEST:
            /*
               ...
               XX.      3 0
               XX.      2 1
             */
            if (!quad[0]->isBorderOpen(Direction::WEST) ||
                !quad[0]->isBorderOpen(Direction::SOUTH))
                return false;
            if (!quad[2]->isBorderOpen(Direction::NORTH) ||
                !quad[2]->isBorderOpen(Direction::EAST))
                return false;
            break;
        case Direction::NORTH_WEST:
            /*
               XX.      2 3
               XX.      1 0
               ...
             */
            if (!quad[0]->isBorderOpen(Direction::NORTH) ||
                !quad[0]->isBorderOpen(Direction::WEST))
                return false;
            if (!quad[2]->isBorderOpen(Direction::EAST) ||
                !quad[2]->isBorderOpen(Direction::SOUTH))
                return false;
            break;
    }
    return true;
}

/**
 * Check rule regarding adjacent cells.
 *
 * @param puzzle    The puzzle state
 * @param route     Route being assessed
 * @return true if adjacency rule is broken
 */
bool adjacencyRuleBroken (ConstPuzzlePtr puzzle, const Route & route)
{
    for (Coordinate coord : route)
    {
        ConstCellPtr pCell = puzzle->getConstCellAtCoordinate(coord);

        // Out of a group of cells covering all directions from current, including diagonal
        // (ie. 3x3 formation, with the current cell in the center),
        // there can be up to 4 formations of 2x2
        std::map<Direction, ConstCellPtr> cellGroup = puzzle->getSurroundingCells(coord);
        cellGroup[Direction::CENTRAL] = pCell;
        /* The 4 possible 2x2 formations are as follows:
           central, north, north east, east;
             . X X
             . X X
             . . .
           central, east, south east, south;
             . . .
             . X X
             . X X
           central, south, south west, west;
             . . .
             X X .
             X X .
           central, west, north west, north;
             X X .
             X X .
             . . .
         */
        // Not sure if there is a better way. Just build formations "manually"
        // Importantly, the array of cells in each quadrant is ordered clockwise.
        std::array<ConstCellPtr, 4> quadrantNorthEast = { cellGroup[Direction::CENTRAL],
            cellGroup[Direction::NORTH], cellGroup[Direction::NORTH_EAST],
            cellGroup[Direction::EAST] };
        std::array<ConstCellPtr, 4> quadrantSouthEast = { cellGroup[Direction::CENTRAL],
            cellGroup[Direction::EAST], cellGroup[Direction::SOUTH_EAST],
            cellGroup[Direction::SOUTH] };
        std::array<ConstCellPtr, 4> quadrantSouthWest = { cellGroup[Direction::CENTRAL],
            cellGroup[Direction::SOUTH], cellGroup[Direction::SOUTH_WEST],
            cellGroup[Direction::WEST] };
        std::array<ConstCellPtr, 4> quadrantNorthWest = { cellGroup[Direction::CENTRAL],
            cellGroup[Direction::WEST], cellGroup[Direction::NORTH_WEST],
            cellGroup[Direction::NORTH] };
        /*std::array<std::array<ConstCellPtr, 4>, 4> twoByTwo = {
                quadrantNorthEast, quadrantSouthEast, quadrantSouthWest, quadrantNorthWest };*/

        if (isAdjacencyLawBroken(quadrantNorthEast, Direction::NORTH_EAST))
        {
#if ANNOUNCE_ADJACENCY_LAW_BREAK
            logger << "Adjacency law broken north-east at " << coord << " in route " << route << std::endl;
            logger.stream() << route << std::endl;
#endif
            return true;
        }
        if (isAdjacencyLawBroken(quadrantSouthEast, Direction::SOUTH_EAST))
        {
#if ANNOUNCE_ADJACENCY_LAW_BREAK
            logger << "Adjacency law broken south-east at " << coord << " in route " << route << std::endl;
            logger.stream() << route << std::endl;
#endif
            return true;
        }
        if (isAdjacencyLawBroken(quadrantSouthWest, Direction::SOUTH_WEST))
        {
#if ANNOUNCE_ADJACENCY_LAW_BREAK
            logger << "Adjacency law broken south-west at " << coord << " in route " << route << std::endl;
            logger.stream() << route << std::endl;
#endif
            return true;
        }
        if (isAdjacencyLawBroken(quadrantNorthWest, Direction::NORTH_WEST))
        {
#if ANNOUNCE_ADJACENCY_LAW_BREAK
            logger << "Adjacency law broken north-west at " << coord << " in route " << route << std::endl;
            logger.stream() << route << std::endl;
#endif
            return true;
        }
    } // for each coordinate
    return false;
}
