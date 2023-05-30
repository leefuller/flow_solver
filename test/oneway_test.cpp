#include "../app/formations.h"
#include "../include/Puzzle.h"
#include "test_helper.h"

bool testOneWayDueToObstructions ()
{
    std::cout << "Test 1 way due to obstructions -------------" << std::endl;
    bool result = true;
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
    if (!check("Test: Route for A can only go UP from {3,0}", theOnlyWay(p, createCoordinate(3,0)) == Direction::NORTH))
        result = false;
    if (!check("Test: Route for B has more than 1 direction from {3,1}", theOnlyWay(p, createCoordinate(3,1)) == Direction::NONE))
        result = false;
    if (!check("Test: Route for B has more than 1 direction from {3,8}", theOnlyWay(p, createCoordinate(3,8)) == Direction::NONE))
        result = false;
    if (!check("Test: Route for A has more than 1 direction from {1,3}", theOnlyWay(p, createCoordinate(1,3)) == Direction::NONE))
        result = false;
    if (result)
        std::cout << "******** Passed one way due to obstructions" << std::endl;
    return result;
}

bool testOneWayDueToCorner ()
{
    std::cout << "Test 1 way due to corner -------------" << std::endl;
    bool result = true;
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
    if (!check("Test route must go right from {5,6}", checkFillToCorner(p, p->getConstCellAtCoordinate(createCoordinate(5,6))) == Direction::EAST))
        result = false;
    // The left side 'B' has to fill to corner, otherwise there would be a dead end
    // or adjacency rule broken
    if (!check("Test route must go left from {5,2}", checkFillToCorner(p, p->getConstCellAtCoordinate(createCoordinate(5,2))) == Direction::WEST))
        result = false;
    if (!check("Test route must go up from {2,7}", checkFillToCorner(p, p->getConstCellAtCoordinate(createCoordinate(2,7))) == Direction::NORTH))
        result = false;
    if (!check("Test route can go more than one way from {1,1}", checkFillToCorner(p, p->getConstCellAtCoordinate(createCoordinate(1,1))) == Direction::NONE))
        result = false;
    if (result)
        std::cout << "******** Passed one way due to corner" << std::endl;
    return result;
}

bool testNotOneWayToObstruction ()
{
    std::cout << "Test not 1 way to obstruction -------------" << std::endl;
    bool result = true;
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
    if (!check("Test should not fill gap",
            checkFillToCorner(p, p->getConstCellAtCoordinate(createCoordinate(2,2))) == Direction::NONE))
        result = false;

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
    if (!check("Test should not fill gap",
            checkFillToCorner(p, p->getConstCellAtCoordinate(createCoordinate(2,2))) == Direction::NONE))
        result = false;

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
    if (!check("Test should not fill gap",
            checkFillToCorner(p, p->getConstCellAtCoordinate(createCoordinate(2,2))) == Direction::NONE))
        result = false;

    if (result)
        std::cout << "******** Passed not one way to obstruction" << std::endl;
    return result;
}

bool testEndpointAdjacent ()
{
    std::cout << "Test endpoint adjacent -------------" << std::endl;
    bool result = true;
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
    if (!check(theOnlyWay(p, createCoordinate(0,6)) == Direction::SOUTH &&
               theOnlyWay(p, createCoordinate(1,6)) == Direction::NORTH))
        result = false;
    std::cout << "******** " << (result ? "Passed" : "FAILED") << " one way due to adjacent endpoint" << std::endl;
    return result;
}

bool testChannel ()
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
    bool result = (check("Test route can go left into channel from {1,5}", theOnlyWay(p, createCoordinate(1,4)) == Direction::WEST));
    std::cout << "******** " << (result ? "Passed" : "FAILED") << " one way due to channel" << std::endl;
    return result;
}

bool testBridgeGap ()
{
    std::cout << "Test bridging gap 1 -------------" << std::endl;
    bool result = true;
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
    if (!check("Test route can go right from {1,2}", theOnlyWay(p, createCoordinate(1,2)) == Direction::EAST))
        return false;
    /*/ TODO this case not implemented
    if (theOnlyWay(p, createCoordinate(1,4)) != Direction::LEFT)
        return false;*/
    // The single space between 'B' and 'C' could be 'B' or 'C'
    if (!check("Test no route at {1,3}", theOnlyWay(p, createCoordinate(3,2)) == Direction::NONE))
        return false;
    if (!check("Test route route from {3,4} has more than 1 choice", theOnlyWay(p, createCoordinate(3,4)) == Direction::NONE))
        return false;
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
    if (!check(theOnlyWay(p, createCoordinate(1,2)) == Direction::EAST))
        return false;
    if (!check(theOnlyWay(p, createCoordinate(1,5)) == Direction::WEST))
        return false;
    // TODO where corner is involved
    std::cout << "******** " << (result ? "Passed" : "FAILED") << " one way to bridge gap" << std::endl;
    return true;

}

int main ()
{
    bool result = true;
    result = result && testOneWayDueToObstructions();
    result = result && testEndpointAdjacent();
    result = result && testChannel();
    result = result && testOneWayDueToCorner();
    result = result && testNotOneWayToObstruction();
    // TODO implementation for bridging the gap
    //result = result && testBridgeGap();
    return result ? 0 : 1;
}
