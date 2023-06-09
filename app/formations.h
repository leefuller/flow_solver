#ifndef INCLUDE_FORMATIONS_H
#define INCLUDE_FORMATIONS_H

#include "../include/Direction.h"
#include "../include/Puzzle.h"
#include <memory>

#define ANNOUNCE_DEAD_END_DETECT        1
#define ANNOUNCE_ONE_WAY_DETECT         1
#define ANNOUNCE_ADJACENCY_LAW_BREAK    1
#define ANNOUNCE_ENTRAPMENT             1
#define ANNOUNCE_ROUTE_FOUND            1
#define ANNOUNCE_SOLVER_DETAIL          0

Direction checkCornerAtCoordinate (ConstPuzzlePtr puzzle, Coordinate c);
Direction checkOneStepToCorner (ConstPuzzlePtr puzzle, Coordinate c, Direction d);

bool detectBadFormation (ConstPuzzlePtr, const Route & route, PipeId id);
bool detectDeadEndFormation (ConstPuzzlePtr puzzle, Coordinate c);
bool detectDeadEndFormation (ConstPuzzlePtr puzzle, const Route & route, PipeId id);
Direction theOnlyWay (ConstPuzzlePtr puzzle, Coordinate c);
Direction checkFillToCorner (ConstPuzzlePtr puzzle, ConstCellPtr pCell);
bool adjacencyRuleBroken (ConstPuzzlePtr puzzle, const Route & route);
bool detectInvalidDeviation (ConstPuzzlePtr puzzle, const Route & route, PipeId id);

#endif
