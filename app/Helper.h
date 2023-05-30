#ifndef HELPER_H
#define HELPER_H

#include <memory>
#include <vector>
#include <set>
#include "../include/Direction.h"
#include "../include/Pipe.h"

class Puzzle;
class Cell;

using ConstPuzzlePtr = std::shared_ptr<const Puzzle>;
using ConstCellPtr = std::shared_ptr<const Cell>;

class Helper
{
  public:
    static bool isCorner (ConstPuzzlePtr puzzle, Coordinate coord) noexcept;

    static std::set<Direction> getObstructedDirections (ConstPuzzlePtr puzzle, Coordinate coord) noexcept;

    static std::vector<ConstCellPtr> getCellsUntilObstruction(ConstPuzzlePtr puzzle, Coordinate, Direction) noexcept;

    static std::set<Direction> getNowTraversableDirections (ConstPuzzlePtr puzzle, Coordinate coord, PipeId idPipe);
    static bool canNowTraverseDirectionFrom (ConstPuzzlePtr puzzle, Coordinate coord, Direction direction, PipeId idPipe) noexcept;
};

#endif
