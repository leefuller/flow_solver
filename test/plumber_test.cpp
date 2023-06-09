#include <gtest/gtest.h>
#include "../include/PuzzleDef.h"
#include "../include/Puzzle.h"
#include "../include/Plumber.h"

static std::shared_ptr<Puzzle> makePuzzle ()
{
    const char * samplePuzzleDef1 = \
    {" = = = = = = = = = ,"  \
     "|. . . . . . . . .|,"  \
     "|                 |,"  \
     "|. . . A . . . . A|,"  \
     "|                 |,"  \
     "|. . . . . . . . .|,"  \
     "|                 |,"  \
     "|. . . . . . . . .|,"  \
     " = = = = = = = = = "   \
    };
    PuzzleDefinition def1(samplePuzzleDef1);
    return def1.generatePuzzle();
}

TEST(plumber_test, testFixtureConnection)
{
    std::shared_ptr<Puzzle> p = makePuzzle();
    std::shared_ptr<Plumber> plumber = p->getPlumber();

    Coordinate from{1,3}, to{1,4};
    plumber->connect(from, to, 'A', CellConnection::FIXTURE_CONNECTION);
    CellPtr pFrom = p->getCellAtCoordinate(from);
    CellPtr pTo = p->getCellAtCoordinate(to);

    EXPECT_TRUE(pFrom->isFixture());
    EXPECT_EQ(pFrom->getConnection(Direction::EAST), CellConnection::FIXTURE_CONNECTION);
    EXPECT_TRUE(pTo->isFixture());
    EXPECT_EQ(pTo->getConnection(Direction::WEST), CellConnection::FIXTURE_CONNECTION);

    EXPECT_EQ(pFrom->getConnection(Direction::EAST), CellConnection::FIXTURE_CONNECTION);
    // Endpoint at "from" should still have available connectors,
    // because the plumber only removes connectors on direct instruction.
    EXPECT_EQ(pFrom->getConnection(Direction::NORTH), CellConnection::OPEN_FIXTURE);
    EXPECT_EQ(pFrom->getConnection(Direction::SOUTH), CellConnection::OPEN_FIXTURE);
    EXPECT_EQ(pFrom->getConnection(Direction::WEST), CellConnection::OPEN_FIXTURE);

    // Endpoint at "to" should still have available open 3 fixture connectors,
    // because the plumber only removes connectors on direct instruction.
    EXPECT_EQ(pTo->getConnection(Direction::NORTH), CellConnection::OPEN_FIXTURE);
    EXPECT_EQ(pTo->getConnection(Direction::SOUTH), CellConnection::OPEN_FIXTURE);
    EXPECT_EQ(pTo->getConnection(Direction::EAST), CellConnection::OPEN_FIXTURE);
    EXPECT_EQ(pTo->getConnection(Direction::WEST), CellConnection::FIXTURE_CONNECTION);

    // Make another connection
    from = {1,4};
    to = {1,5};
    plumber->connect(from, to, 'A', CellConnection::FIXTURE_CONNECTION);
    pFrom = p->getCellAtCoordinate(from);
    pTo = p->getCellAtCoordinate(to);
    EXPECT_EQ(pFrom->getConnection(Direction::EAST), CellConnection::FIXTURE_CONNECTION);
    EXPECT_EQ(pFrom->getConnection(Direction::NORTH), CellConnection::OPEN_FIXTURE);
    EXPECT_EQ(pFrom->getConnection(Direction::SOUTH), CellConnection::OPEN_FIXTURE);
    EXPECT_EQ(pFrom->getConnection(Direction::WEST), CellConnection::FIXTURE_CONNECTION);
    EXPECT_EQ(pTo->getConnection(Direction::NORTH), CellConnection::OPEN_FIXTURE);
    EXPECT_EQ(pTo->getConnection(Direction::SOUTH), CellConnection::OPEN_FIXTURE);
    EXPECT_EQ(pTo->getConnection(Direction::EAST), CellConnection::OPEN_FIXTURE);
    EXPECT_EQ(pTo->getConnection(Direction::WEST), CellConnection::FIXTURE_CONNECTION);


    from = {1,3};
    to = {1,4};
    pFrom = p->getCellAtCoordinate(from);
    pTo = p->getCellAtCoordinate(to);
    plumber->removeConnector(pFrom, Direction::NORTH);
    EXPECT_EQ(pFrom->getConnection(Direction::NORTH), CellConnection::NO_CONNECTOR);
    EXPECT_EQ(pFrom->getConnection(Direction::SOUTH), CellConnection::OPEN_FIXTURE);
    EXPECT_EQ(pFrom->getConnection(Direction::EAST), CellConnection::OPEN_FIXTURE);
    EXPECT_EQ(pFrom->getConnection(Direction::WEST), CellConnection::OPEN_FIXTURE);

    plumber->removeConnector(pFrom, Direction::SOUTH);
    plumber->removeConnector(pFrom, Direction::WEST);
    // Should not be able to remove a fixed connection
    //plumber->removeConnector(from, Direction::EAST);

    pFrom = p->getCellAtCoordinate(from);
    pTo = p->getCellAtCoordinate(to);

    // TODO more
    //bool plumber->removeConnector(CellPtr pCell, Direction d);
}

TEST(plumber_test, testRemoveConnector)
{
    // TODO
}

TEST(plumber_test, testExceptions)
{
    // TODO
}
