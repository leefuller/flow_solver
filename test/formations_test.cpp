#include <iostream>

#include "../app/formations.h"
#include "../app/Solver.h"
#include <gtest/gtest.h>

/*TEST(formations_test, testInvalidDeviation)
{
    // TODO formation test for invalid deviation
}*/

TEST(formations_test, testEntrapment)
{
    const char * def = \
    {" = = = = = = ,"  \
     "|. . . . B A|,"  \
     "|           |,"  \
     "|. . A . . .|,"  \
     "|           |,"  \
     "|. . . B . .|,"  \
     " = = = = = = "   \
    };
    PuzzleDefinition def1(def);
    std::shared_ptr<Puzzle> p1 = def1.generatePuzzle();

    Route route = {};
    EXPECT_FALSE(detectEntrapment(p1, route, 'B'));

    route = {{0,4}};
    {
        TryRoute r(p1, 'B', route); // plug route into puzzle
        EXPECT_FALSE(detectEntrapment(p1, route, 'B'));
    }

    route = {{0,4},{1,4},{2,4},{2,3}}; // route traps 'A'
    {
        TryRoute r(p1, 'B', route); // plug route into puzzle
        EXPECT_TRUE(detectEntrapment(p1, route, 'B'));
    }

    // Trap using partial route
    route = {{0,4},{1,4}}; // Partial B route traps 'A'
    {
        TryRoute r(p1, 'B', route); // plug route into puzzle
        EXPECT_TRUE(detectEntrapment(p1, route, 'B'));
    }
}

/*TEST(formations_test, testCornerCheck)
{
    // TODO formation test for corner check
}*/
