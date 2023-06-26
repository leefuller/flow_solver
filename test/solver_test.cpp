#include <iostream>

#include "../include/Puzzle.h"
#include "../app/Solver.h"
#include <gtest/gtest.h>

TEST(solver_test, test_puzzle1)
{
    const char * def = \
    {" = = = = = = = ,"  \
    "|. . . . . . .|,"  \
    "|             |,"  \
    "|A B . . . B A|,"  \
    " = = = = = = = "   \
    };
    Solver solver(def);
    EXPECT_TRUE(solver.solve());
}

TEST(solver_test, test_puzzle2)
{
    const char * def = \
    {" = = = = = = ,"  \
    "|. . R Y . .|,"  \
    "|           |,"  \
    "|. . B O G .|,"  \
    "|           |,"  \
    "|. . O . . .|,"  \
    "|           |,"  \
    "|R . G . . .|,"  \
    "|           |,"  \
    "|Y . . . B .|,"  \
    "|           |,"  \
    "|. . . . . .|,"  \
    " = = = = = = "   \
    };
    Solver solver(def);
    EXPECT_TRUE(solver.solve());
}

TEST(solver_test, test_courtyard1)
{
const char * courtyard1 = \
{" = = = = = = = ,"  \
 "|. . . . . . .|,"  \
 "|  ===   ===  |,"  \
 "|.|. . . . .|.|,"  \
 "| |         | |,"  \
 "|G|. Y B . .|.|,"  \
 "|             |,"  \
 "|. . . Y . . .|,"  \
 "|             |,"  \
 "|.|B . . . .|.|,"  \
 "| |         | |,"  \
 "|R|R . . . .|.|,"  \
 "|  ===   ===  |,"  \
 "|G . . . . . .|,"  \
 " = = = = = = ="   \
};
    Solver solver(courtyard1);
    EXPECT_TRUE(solver.solve());
}

TEST(solver_test, test_pockets34)
{
const char * pockets34 =
 //0 1 2 3 4 5 6 7 8
{" = = = = = = = = = ,"  \
 "|. . P N . . . . .|,"  \
 "|                 |,"  \
 "|.|R|. . .|G|. . .|,"  \
 "|  =       =      |,"  \
 "|. . . . . . .|N|.|,"  \
 "|              =  |,"  \
 "|. . .|P|. . . . .|,"  \
 "|      =          |,"  \
 "|.|O|. . .|Y|. . .|,"  \
 "|  =       =      |,"  \
 "|. R . . . . .|B|.|,"  \
 "|              =  |,"  \
 "|. . . .|G|Y . . .|,"  \
 "|        =        |,"  \
 "|. .|K|. . .|K|. .|,"  \
 "|    =       =    |,"  \
 "|O . . . . . . . B|,"  \
 " = = = = = = = = = "   \
};
    Solver solver(pockets34);
    EXPECT_TRUE(solver.solve());
}

TEST(solver_test, test_puzzle5)
{
const char * puzzleDef3 = \
{" =================== ,"  \
 "|. Y|     |. . . . .|,"  \
 "|  =   =  |         |,"  \
 "|.|   |R| |p . b O .|,"  \
 "| |  =  | |=        |,"  \
 "|.| |. .|   |b . . .|,"  \
 "| | |    =| |=   =  |,"  \
 "|.| |. . O|   |.| |p|,"  \
 "| | |      =  | | | |,"  \
 "|.| |. . . G| |.| |Y|,"  \
 "| | |      =  | | | |,"  \
 "|.| |. . G|   |.| |.|,"  \
 "| | |      ===  | | |,"  \
 "|.| |. . . . . .| |.|,"  \
 "| | |    =======  | |,"  \
 "|.| |. R|         |.|,"  \
 "| |  ===   =======  |,"  \
 "|.|       |. . . . .|,"  \
 "|  =======          |,"  \
 "|. . . . . . P . . P|,"  \
 " =================== "   \
};
    Solver solver(puzzleDef3);
    EXPECT_TRUE(solver.solve());
}

TEST(solver_test, test_inkblot1)
{
const char * inkblot1 = \
{" =============== ,"  \
 "|G .|       |. G|,"  \
 "|    =     =    |,"  \
 "|R . .|   |. . B|,"  \
 "|      ===      |,"  \
 "|. O . . . . Y .|,"  \
 "|    =======    |,"  \
 "|. .|       |. B|,"  \
 "|   |       |   |,"  \
 "|. .|       |. .|,"  \
 "|   |       |   |,"  \
 "|R .|       |b Y|,"  \
 "|   |       |   |,"  \
 "|. .|       |. .|,"  \
 "|  =         =  |,"  \
 "|O|           |b|,"  \
 " =============== "   \
};
    Solver solver(inkblot1);
    EXPECT_TRUE(solver.solve());
}

TEST(solver_test, test_puzzle7)
{
const char * _8x8_Mania1 = \
{" = = = = = = = = ,"  \
 "|B . . B Y . . .|,"  \
 "|               |,"  \
 "|. . R P K . K .|,"  \
 "|               |,"  \
 "|. . D . . . . .|,"  \
 "|               |,"  \
 "|. . G . O N . .|,"  \
 "|               |,"  \
 "|. . . . P . . .|,"  \
 "|               |,"  \
 "|. . . R . . . .|,"  \
 "|               |,"  \
 "|. D . G . . O Y|,"  \
 "|               |,"  \
 "|. . . . . . . N|,"  \
 " = = = = = = = ="   \
};
    Solver solver(_8x8_Mania1);
    EXPECT_TRUE(solver.solve());
}

#if 1
TEST(solver_test, test_worm31)
{
const char * worm31 =
 //0 1 2 3 4 5 6 7 8 9 10
{" ===================== ,"  \
 "|N .|     |O . . . . .|,"  \
 "|   |  =   =          |,"  \
 "|G .| |B|   |P . . . .|,"  \
 "|   | |  ===          |,"  \
 "|. .| |. . . . . . . .|,"  \
 "|   | |               |,"  \
 "|. .| |B . O . b . . .|,"  \
 "|   |  =              |,"  \
 "|. .|   |. W . . . P .|,"  \
 "|    ===              |,"  \
 "|. . Y . . Y . b W . .|,"  \
 "|                     |,"  \
 "|. . . . . . . . p . N|,"  \
 "|                     |,"  \
 "|. . . p . . . . . . .|,"  \
 "|                     |,"  \
 "|. . . . . . . . . R .|,"  \
 "|                     |,"  \
 "|. . R . . . . . . . .|,"  \
 "|                     |,"  \
 "|G . . . . . . . . . .|,"  \
 " ===================== "   \
};
    Solver solver(worm31);
    EXPECT_TRUE(solver.solve());
}
#endif

TEST(solver_test, test_extreme22)
{
const char * extreme22 = \
{" = = = = = = = = ,"  \
 "|. . . . . . . .|,"  \
 "|               |,"  \
 "|. B . . . . . .|,"  \
 "|               |,"  \
 "|. . . G . . . .|,"  \
 "|               |,"  \
 "|. . . . . . . .|,"  \
 "|               |,"  \
 "|. . . R . . . .|,"  \
 "|               |,"  \
 "|. . B . . . . .|,"  \
 "|               |,"  \
 "|Y . . . . . . .|,"  \
 "|               |,"  \
 "|. . . . . G Y R|,"  \
 " = = = = = = = ="   \
};
    Solver solver(extreme22);
    EXPECT_TRUE(solver.solve());
}