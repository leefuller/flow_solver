#include "formations.h"
#include "../include/Puzzle.h"
#include "../include/PuzzleException.h"
#include "Logger.h"

static Logger & logger = Logger::getDefaultLogger();

/*
 * The adjacency law says that cells in a 2x2 formation without a dividing wall
 * cannot have the same pipe,
 *
 * This is not a rule of the puzzle, but no solution has such a formation.
 *
 * The adjacency rule is intended to eliminate a route containing a formation like these:
 * X X X       X X X
 *   X X         X|X
 * (The formations are invalid because the route could just be X X X. There is no need for diversion.)
 *
 * A formation with separating wall like shown here is valid:
 * X X|X     or  X X|X
 *   X|X           X X
 */


/**
 * Check rule regarding adjacent cells.
 *
 * @param puzzle    The puzzle state
 * @param coord     Coordinate for cell being assessed, which is treated as top left in potential square
 * @param inwardDirection   The side from which the cell is entered.
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
        std::array<ConstCellPtr, 9> cellGroup = puzzle->getAdjacentCells(coord);
        /* The 4 possible 2x2 formations are as follows:
           central, up, up right, right;
             . X X
             . X X
             . . .
           central, down, right, down right;
             . . .
             . X X
             . X X
           central, down, down left, left;
             . . .
             X X .
             X X .
           central, up, up left, left;
             X X .
             X X .
             . . .
         */
        // Not sure if there is a better way. Just build formations "manually"
        // Importantly (for later logic) the array of cells in each quadrant is ordered clockwise,
        // so that the first cell in each array is the one shared with the previous one.
        // The upper pair share UP (12 oclock)
        // The right pair share RIGHT (3 oclock)
        // The down pair share DOWN (6 oclock)
        // The left pair share LEFT (9 oclock)
        // This means:
        // The 1st cell and 3rd cell in each array below share a border with the center cell.
        // The 2nd cell in each array is the one diagonally opposite the center cell.
        std::array<ConstCellPtr, 3> quadrantUpRight = {
            cellGroup[Adjacency::ADJACENT_NORTH], cellGroup[Adjacency::ADJACENT_NORTH_EAST],
            cellGroup[Adjacency::ADJACENT_EAST] };
        std::array<ConstCellPtr, 3> quadrantDownRight = {
            cellGroup[Adjacency::ADJACENT_EAST], cellGroup[Adjacency::ADJACENT_SOUTH_EAST],
            cellGroup[Adjacency::ADJACENT_SOUTH] };
        std::array<ConstCellPtr, 3> quadrantDownLeft = {
            cellGroup[Adjacency::ADJACENT_SOUTH], cellGroup[Adjacency::ADJACENT_SOUTH_WEST],
            cellGroup[Adjacency::ADJACENT_WEST] };
        std::array<ConstCellPtr, 3> quadrantUpLeft = {
            cellGroup[Adjacency::ADJACENT_WEST], cellGroup[Adjacency::ADJACENT_NORTH_WEST],
            cellGroup[Adjacency::ADJACENT_NORTH] };
        std::array<std::array<ConstCellPtr, 3>, 4> twoByTwo = {
                quadrantUpRight, quadrantDownRight, quadrantDownLeft, quadrantUpLeft };

        // For each potential 2x2 square, there is no adjacency issue,
        // if any are nullptr, or if any are empty, or if not all have the same pipe.
        // We already have the central cell, and know it is not empty. So only 3 adjacent to check.
        // The array initialization above was ordered to that progression starts with the first cell
        // of the first array corresponding to 12 oclock.
        if (pCell->getPipeId() == NO_PIPE_ID)
        {
#if ANNOUNCE_ADJACENCY_LAW_BREAK
            logger << "Cell unexpectedly empty at " << coord << std::endl;
#endif
            throw PuzzleException("expected pipe for adjacency check");
        }
        unsigned pos = 0;
        for (std::array<ConstCellPtr, 3> & a : twoByTwo)
        {
            bool issue = true;
            for (ConstCellPtr p : a)
            {
                if (p == nullptr)
                {
                    issue = false;
                    break;
                }
                if (p->getPipeId() != pCell->getPipeId()) // covers check for empty adjacent
                {
                    issue = false;
                    break;
                }
            }
            if (issue) // The 2x2 group is all for the same pipe.
            {
                // Check for inner wall. There can be up to 4 inner walls in 2x2 group.
                // 2 inner walls can be detected at the central cell.
                // The other 2 can be detected at the cell diagonally opposite the central cell.
                // The 2nd cell in each array is the one diagonally opposite the center cell.
                switch (pos)
                {
                    case 0: // upper right
                        if (pCell->getBorder(Direction::UP) != CellBorder::WALL &&
                                pCell->getBorder(Direction::RIGHT) != CellBorder::WALL &&
                                a[1]->getBorder(Direction::LEFT) != CellBorder::WALL &&
                                a[1]->getBorder(Direction::DOWN) != CellBorder::WALL)
                            return true;
                        break;
                    case 1: // lower right
                        if (pCell->getBorder(Direction::DOWN) != CellBorder::WALL &&
                                pCell->getBorder(Direction::RIGHT) != CellBorder::WALL &&
                                a[1]->getBorder(Direction::LEFT) != CellBorder::WALL &&
                                a[1]->getBorder(Direction::UP) != CellBorder::WALL)
                            return true;
                        break;
                    case 2: // lower left
                        if (pCell->getBorder(Direction::DOWN) != CellBorder::WALL &&
                                pCell->getBorder(Direction::LEFT) != CellBorder::WALL &&
                                a[1]->getBorder(Direction::RIGHT) != CellBorder::WALL &&
                                a[1]->getBorder(Direction::UP) != CellBorder::WALL)
                            return true;
                        break;
                    case 3: // upper left
                        if (pCell->getBorder(Direction::UP) != CellBorder::WALL &&
                                pCell->getBorder(Direction::LEFT) != CellBorder::WALL &&
                                a[1]->getBorder(Direction::RIGHT) != CellBorder::WALL &&
                                a[1]->getBorder(Direction::DOWN) != CellBorder::WALL)
                            return true;
                        break;
                }
                issue = false;
            }
            ++pos;
        }
    } // for each coordinate
    return false;
}
