#ifndef INCLUDE_FORMATIONS_H
#define INCLUDE_FORMATIONS_H

#include "../include/Direction.h"
#include "../include/Puzzle.h"
#include <memory>

#define ANNOUNCE_DEAD_END_DETECT        0
#define ANNOUNCE_ONE_WAY_DETECT         1
#define ANNOUNCE_ADJACENCY_LAW_BREAK    0

bool detectBadFormation (ConstPuzzlePtr, const Route & route, PipeId id);
bool detectDeadEndFormation (ConstPuzzlePtr puzzle, const Route & route, PipeId id);
Direction theOnlyWay (ConstPuzzlePtr puzzle, Coordinate c);
bool adjacencyRuleBroken (ConstPuzzlePtr puzzle, const Route & route);
bool detectInvalidDeviation (ConstPuzzlePtr puzzle, const Route & route, PipeId id);

#endif
