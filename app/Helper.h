#ifndef HELPER_H
#define HELPER_H

#include <memory>
#include <vector>
#include <set>
#include "../include/Direction.h"
#include "../include/Pipe.h"

class Puzzle;
class Cell;

class Helper
{
  public:
    static bool isCorner (std::shared_ptr<const Puzzle> puzzle, Coordinate coord) noexcept;

    static unsigned getObstructedDirections (std::shared_ptr<const Puzzle> puzzle, std::shared_ptr<const Cell> pCell, std::set<Direction> & walls, std::set<Direction> & pipes) noexcept;
    static std::vector<std::shared_ptr<const Cell>> getCellsUntilObstruction(std::shared_ptr<const Puzzle> puzzle, Coordinate, Direction) noexcept;

    static std::set<Direction> getNowTraversableDirections (std::shared_ptr<const Puzzle> puzzle, Coordinate coord, PipeId idPipe);
    static bool canNowTraverseDirectionFrom (std::shared_ptr<const Puzzle> puzzle, Coordinate coord, Direction direction, PipeId idPipe) noexcept;
};

#endif
