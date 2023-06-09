#include "../app/formations.h"
#include "../include/Puzzle.h"
#include <gtest/gtest.h>

TEST(deadend_test, deadend_for_coordinate)
{
    const char * samplePuzzleDef1 = \
    {" = = = = = = = ,"  \
     "|. .|. . . . A|,"  \
     "|  =          |,"  \
     "|. . . A . . .|,"  \
     " = = = = = = = "   \
    };
    PuzzleDefinition def1(samplePuzzleDef1);
    std::shared_ptr<Puzzle> p1 = def1.generatePuzzle();

    EXPECT_FALSE(detectDeadEndFormation(p1, {0,0}));
    EXPECT_TRUE(detectDeadEndFormation(p1, {0,1}));

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
    EXPECT_FALSE(detectDeadEndFormation(p2, {0,1}));
}

TEST(deadend_test, deadend_for_route)
{

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
    EXPECT_TRUE(detectDeadEndFormation(p1, route, 'A'));
}
