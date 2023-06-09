#include "../app/formations.h"
#include "../include/Puzzle.h"
#include <gtest/gtest.h>

TEST(oneway_test, testOneWayDueToObstructions)
{
    std::cout << "Test 1 way due to obstructions -------------" << std::endl;
    const char * samplePuzzleDef1 = \
    {" = = = = = = = = = ,"  \
     "|. . . . . . . . .|,"  \
     "|                 |,"  \
     "|. . . A . . . . .|,"  \
     "|                 |,"  \
     "|. . . . . . . . .|,"  \
     "|                 |,"  \
     "|A B . . . . . . B|,"  \
     " = = = = = = = = = "   \
    };
    // Lower left 'A' must go UP because obstructed in the other 3 directions.
    PuzzleDefinition def1(samplePuzzleDef1);
    std::shared_ptr<Puzzle> p = def1.generatePuzzle();
    p->streamPuzzleMatrix(std::cout);
    EXPECT_TRUE(theOnlyWay(p, createCoordinate(3,0)) == Direction::NORTH);
    EXPECT_TRUE(theOnlyWay(p, createCoordinate(3,1)) == Direction::NONE);
    EXPECT_TRUE(theOnlyWay(p, createCoordinate(3,8)) == Direction::NONE);
    EXPECT_TRUE(theOnlyWay(p, createCoordinate(1,3)) == Direction::NONE);
}

TEST(oneway_test, testOneWayDueToCorner)
{
    std::cout << "Test 1 way due to corner -------------" << std::endl;
    const char * samplePuzzleDef2 = \
    {" = = = = = = = = ,"  \
     "|. . . . . . . .|,"  \
     "|               |,"  \
     "|. A . . . . . .|,"  \
     "|               |,"  \
     "|. . . . . . . A|,"  \
     "|               |,"  \
     "|. . . . . . . .|,"  \
     "|               |,"  \
     "|. . . . . . . .|,"  \
     "|               |,"  \
     "|. . B . . . B .|,"  \
     " = = = = = = = = "   \
    };
    // Because the bottom right corner would be a dead end for any pipe other than 'B'
    // there is only 1 way for the right side 'B' to go (RIGHT)
    PuzzleDefinition def2(samplePuzzleDef2);
    std::shared_ptr<Puzzle> p = def2.generatePuzzle();
    EXPECT_TRUE(checkFillToCorner(p, p->getConstCellAtCoordinate(createCoordinate(5,6))) == Direction::EAST);
    // The left side 'B' has to fill to corner, otherwise there would be a dead end
    // or adjacency rule broken
    EXPECT_TRUE(checkFillToCorner(p, p->getConstCellAtCoordinate(createCoordinate(5,2))) == Direction::WEST);
    EXPECT_TRUE(checkFillToCorner(p, p->getConstCellAtCoordinate(createCoordinate(2,7))) == Direction::NORTH);
    EXPECT_TRUE(checkFillToCorner(p, p->getConstCellAtCoordinate(createCoordinate(1,1))) == Direction::NONE);
}

TEST(oneway_test, testNotOneWayToObstruction)
{
    std::cout << "Test not 1 way to obstruction -------------" << std::endl;
    // Early days saw algorithm fill between 'Y' endpoints, when it should not.
    const char * samplePuzzleDef1 = \
    {" = = = = = = = = ,"  \
     "|. . . . . . . .|,"  \
     "|               |,"  \
     "|. . . . . . . .|,"  \
     "|    = =        |,"  \
     "|. . Y . . Y . .|,"  \
     "|               |,"  \
     "|. . . . . . . .|,"  \
     "|               |,"  \
     "|. . . . . . . .|,"  \
     "|               |,"  \
     "|. . . . . . . .|,"  \
     " = = = = = = = = "   \
    };
    PuzzleDefinition def1(samplePuzzleDef1);
    std::shared_ptr<Puzzle> p = def1.generatePuzzle();
    EXPECT_TRUE(checkFillToCorner(p, p->getConstCellAtCoordinate(createCoordinate(2,2))) == Direction::NONE);

    const char * samplePuzzleDef2 = \
    {" = = = = = = = = ,"  \
     "|. . . . . . . .|,"  \
     "|               |,"  \
     "|. . . . . . . .|,"  \
     "|               |,"  \
     "|. . . . . X . .|,"  \
     "|               |,"  \
     "|X . . . . . . .|,"  \
     "|               |,"  \
     "|. . . . Y . . .|,"  \
     "|               |,"  \
     "|. . . . . . . .|,"  \
     "|               |,"  \
     "|Y . . . . . . .|,"  \
     " = = = = = = = = "   \
    };
    PuzzleDefinition def2(samplePuzzleDef2);
    p = def2.generatePuzzle();
    EXPECT_TRUE(checkFillToCorner(p, p->getConstCellAtCoordinate(createCoordinate(2,2))) == Direction::NONE);

    // TODO this test is invalid once the "bridge the gap" algorithm is done.
    const char * samplePuzzleDef3 = \
    {" = = = = = = = = ,"  \
     "|. . . . . . . .|,"  \
     "|               |,"  \
     "|. . . . . . . .|,"  \
     "|               |,"  \
     "|. . . . . Y . .|,"  \
     "|               |,"  \
     "|X . . . . . . .|,"  \
     "|               |,"  \
     "|. . . . Y . . .|,"  \
     "|               |,"  \
     "|. . . . . . . .|,"  \
     "|               |,"  \
     "|X . . . . . . .|,"  \
     " = = = = = = = = "   \
    };
    PuzzleDefinition def3(samplePuzzleDef2);
    p = def3.generatePuzzle();
    EXPECT_TRUE(checkFillToCorner(p, p->getConstCellAtCoordinate(createCoordinate(2,2))) == Direction::NONE);
}

TEST(oneway_test, testEndpointAjacent)
{
    std::cout << "Test endpoint adjacent -------------" << std::endl;
    const char * samplePuzzleDef1 = \
    {" = = = = = = = ,"  \
     "|. . . A . . B|,"  \
     "|             |,"  \
     "|A . . . . . B|,"  \
     " = = = = = = = "   \
    };
    // Although above is an invalid puzzle,
    // we are only interested in cells where a pipe can only go one way.
    PuzzleDefinition def1(samplePuzzleDef1);
    std::shared_ptr<Puzzle> p = def1.generatePuzzle();
    // Don't know which one is end vs start, so check both
    EXPECT_TRUE((theOnlyWay(p, createCoordinate(0,6)) == Direction::SOUTH &&
               theOnlyWay(p, createCoordinate(1,6)) == Direction::NORTH));
}

TEST(oneway_test, testChannel)
{
    std::cout << "Test channel -------------" << std::endl;
    const char * samplePuzzleChannel1 = \
    {" = = = = = = = ,"  \
     "|. . . . . . .|,"  \
     "|    = =      |,"  \
     "|. . . . A . .|,"  \
     "|    = =      |,"  \
     "|. . . . . . A|,"  \
     " = = = = = = = "   \
    };
    // One 'A' is blocking a channel, so can only go into it
    PuzzleDefinition def1(samplePuzzleChannel1);
    std::shared_ptr<Puzzle> p = def1.generatePuzzle();
    EXPECT_TRUE(theOnlyWay(p, createCoordinate(1,4)) == Direction::WEST);
}

TEST(oneway_test, testBridgeGap)
{
    std::cout << "Test bridging gap 1 -------------" << std::endl;
    const char * samplePuzzleGap1 = \
    {" = = = = = = = = ,"  \
     "|. . . . . . . .|,"  \
     "|               |,"  \
     "|. . A . A . . .|,"  \
     "|    = =        |,"  \
     "|. . . . . . . .|,"  \
     "|               |,"  \
     "|. . B . C . . .|,"  \
     "|    = =        |,"  \
     "|B . . . . . . C|,"  \
     " = = = = = = = = "   \
    };
    // The space between 'A' endpoints must be 'A'
    PuzzleDefinition def1(samplePuzzleGap1);
    std::shared_ptr<Puzzle> p = def1.generatePuzzle();
    EXPECT_TRUE(theOnlyWay(p, createCoordinate(1,2)) == Direction::EAST);
    /*/ TODO this case not implemented
    EXPECT_TRUE(theOnlyWay(p, createCoordinate(1,4)) != Direction::LEFT);*/
    // The single space between 'B' and 'C' could be 'B' or 'C'
    EXPECT_TRUE(theOnlyWay(p, createCoordinate(3,2)) == Direction::NONE);
    EXPECT_TRUE(theOnlyWay(p, createCoordinate(3,4)) == Direction::NONE);
    std::cout << "Test bridging gap 2 -------------" << std::endl;
    const char * samplePuzzleGap2 = \
    {" = = = = = = = ,"  \
     "|. . . . . . .|,"  \
     "|             |,"  \
     "|. . A . . A .|,"  \
     "|    = = =    |,"  \
     "|. . . . . . .|,"  \
     "|             |,"  \
     "|. . . . . . .|,"  \
     "|             |,"  \
     "|. . . . . . .|,"  \
     " = = = = = = = "   \
    };
    // The spaces between 'A' endpoints must be 'A'
    PuzzleDefinition def2(samplePuzzleGap2);
    p = def2.generatePuzzle();
    EXPECT_TRUE(theOnlyWay(p, createCoordinate(1,2)) == Direction::EAST);
    EXPECT_TRUE(theOnlyWay(p, createCoordinate(1,5)) == Direction::WEST);
    // TODO where corner is involved
}
