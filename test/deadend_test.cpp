#include "../app/formations.h"
#include "../include/Puzzle.h"
#include "test_helper.h"

bool testDeadEndForCoordinate ()
{
    bool result = true;

    const char * samplePuzzleDef1 = \
    {" = = = = = = = ,"  \
     "|. .|. . . . A|,"  \
     "|  =          |,"  \
     "|. . . A . . .|,"  \
     " = = = = = = = "   \
    };
    PuzzleDefinition def1(samplePuzzleDef1);
    std::shared_ptr<Puzzle> p1 = def1.generatePuzzle();

    result = result && check("Check no dead end at {0,0}", !detectDeadEndFormation(p1, {0,0}));
    result = result && check("Check dead end at {0,1}", detectDeadEndFormation(p1, {0,1}));

    // Put a pipe in the dead end. Will be an endpoint, so no longer a dead end
    const char * samplePuzzleDef2 = \
    {" = = = = = = = ,"  \
     "|. A|. . . . A|,"  \
     "|  =          |,"  \
     "|. . . . . . .|,"  \
     " = = = = = = = "   \
    };
    PuzzleDefinition def2(samplePuzzleDef2);
    std::shared_ptr<Puzzle> p2 = def2.generatePuzzle();
    result = result && check("Check no dead end at endpoint {0,1}", !detectDeadEndFormation(p2, {0,1}));

    return result;
}

bool testDeadEndForRoute ()
{
    bool result = true;

    const char * samplePuzzleDef1 = \
    {" = = = = = = = ,"  \
     "|. . A . . . A|,"  \
     "|             |,"  \
     "|. . . . . . .|,"  \
     " = = = = = = = "   \
    };
    PuzzleDefinition def1(samplePuzzleDef1);
    std::shared_ptr<Puzzle> p1 = def1.generatePuzzle();

    // Create route causing adjacent dead end
    Route route = {{0,2},{0,3},{1,3},{1,4},{1,5},{0,5},{0,6}};
    for (Coordinate c : route)
    {
        CellPtr pc = p1->getCellAtCoordinate(c);
        pc->setPipeId('A');
    }

    result = result && check("Check dead end due to route", detectDeadEndFormation(p1, route, 'A'));
    return result;
}

int main ()
{
    bool result = true;
    result = result && testDeadEndForCoordinate();
    result = result && testDeadEndForRoute();
    return result ? 0 : 1;
}
