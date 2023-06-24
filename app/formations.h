#ifndef INCLUDE_FORMATIONS_H
#define INCLUDE_FORMATIONS_H

#include "../include/Direction.h"
#include "../include/Puzzle.h"
#include <memory>

#define ANNOUNCE_DEAD_END_DETECT        0
#define ANNOUNCE_ONE_WAY_DETECT         0
#define ANNOUNCE_ADJACENCY_LAW_BREAK    0
#define ANNOUNCE_ENTRAPMENT             0
#define ANNOUNCE_ROUTE_FOUND            0
#define ANNOUNCE_SOLVER_DETAIL          0

bool isCorner (ConstPuzzlePtr puzzle, Coordinate coord) noexcept;

Direction checkCornerAtCoordinate (ConstPuzzlePtr puzzle, Coordinate c);
Direction checkOneStepToCorner (ConstPuzzlePtr puzzle, Coordinate c, Direction d);

bool detectBadFormation (ConstPuzzlePtr, const Route & route, PipeId id);
bool detectDeadEndFormation (ConstPuzzlePtr puzzle, Coordinate c);
bool detectDeadEndFormation (ConstPuzzlePtr puzzle, const Route & route, PipeId id);
Direction theOnlyWay (ConstPuzzlePtr puzzle, Coordinate c);
Direction checkFillToCorner (ConstPuzzlePtr puzzle, ConstCellPtr pCell);
bool adjacencyRuleBroken (ConstPuzzlePtr puzzle, const Route & route);
bool detectInvalidDeviation (ConstPuzzlePtr puzzle, const Route & route, PipeId id);
bool detectEntrapment (ConstPuzzlePtr puzzle, const Route & route, PipeId idPipe);

#endif
