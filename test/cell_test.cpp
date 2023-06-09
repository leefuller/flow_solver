#include <iostream>

#include "../include/Cell.h"
#include <gtest/gtest.h>

TEST(cell_test, test_border_ops)
{
    CellBorder edge = CellBorder::OPEN;
    EXPECT_EQ(toggleBorder(edge), CellBorder::WALL);
    EXPECT_EQ(edge, CellBorder::OPEN);
    edge = CellBorder::WALL;
    EXPECT_EQ(toggleBorder(edge), CellBorder::OPEN);
    EXPECT_EQ(edge, CellBorder::WALL);

    bool eq;
    eq = (getOppositeBorder(
            std::array<CellBorder, 4>({CellBorder::OPEN, CellBorder::OPEN, CellBorder::OPEN, CellBorder::OPEN})) ==
            std::array<CellBorder, 4>({CellBorder::WALL, CellBorder::WALL, CellBorder::WALL, CellBorder::WALL}));
    EXPECT_TRUE(eq);
    eq = (getOppositeBorder(
            std::array<CellBorder, 4>({CellBorder::WALL, CellBorder::OPEN, CellBorder::OPEN, CellBorder::OPEN})) ==
            std::array<CellBorder, 4>({CellBorder::OPEN, CellBorder::WALL, CellBorder::WALL, CellBorder::WALL}));
    EXPECT_TRUE(eq);
    eq = (getOppositeBorder(
            std::array<CellBorder, 4>({CellBorder::OPEN, CellBorder::WALL, CellBorder::OPEN, CellBorder::OPEN})) ==
            std::array<CellBorder, 4>({CellBorder::WALL, CellBorder::OPEN, CellBorder::WALL, CellBorder::WALL}));
    EXPECT_TRUE(eq);
    eq = (getOppositeBorder(
            std::array<CellBorder, 4>({CellBorder::OPEN, CellBorder::OPEN, CellBorder::WALL, CellBorder::OPEN})) ==
            std::array<CellBorder, 4>({CellBorder::WALL, CellBorder::WALL, CellBorder::OPEN, CellBorder::WALL}));
    EXPECT_TRUE(eq);
    eq = (getOppositeBorder(
            std::array<CellBorder, 4>({CellBorder::OPEN, CellBorder::OPEN, CellBorder::OPEN, CellBorder::WALL})) ==
            std::array<CellBorder, 4>({CellBorder::WALL, CellBorder::WALL, CellBorder::WALL, CellBorder::OPEN}));
    EXPECT_TRUE(eq);
}

void todo ()
{
    //Cell cell; // private
    // TODO more cell tests
    /*
    result = result && check(cell.getPipeId() == NO_PIPE_ID);
    result = result && check(cell.getBorder(Direction::UP) == CellBorder::OPEN);
    result = result && check(cell.getBorder(Direction::DOWN) == CellBorder::OPEN);
    result = result && check(cell.getBorder(Direction::LEFT) == CellBorder::OPEN);
    result = result && check(cell.getBorder(Direction::RIGHT) == CellBorder::OPEN);
    result = result && check(cell.getBorders() == std::array<CellBorder, 4>({CellBorder::OPEN, CellBorder::OPEN, CellBorder::OPEN, CellBorder::WALL}));
    result = result && check(cell.getConnection(Direction::UP) == CellConnection::NO_CONNECTOR);
    result = result && check(cell.getConnection(Direction::DOWN) == CellConnection::NO_CONNECTOR);
    result = result && check(cell.getConnection(Direction::LEFT) == CellConnection::NO_CONNECTOR);
    result = result && check(cell.getConnection(Direction::RIGHT) == CellConnection::NO_CONNECTOR);
    result = result && check(cell.getConnections() == std::array<CellConnection, 4>({CellConnection::NO_CONNECTOR,CellConnection::NO_CONNECTOR,CellConnection::NO_CONNECTOR,CellConnection::NO_CONNECTOR}));
    */
    //setPipeId()
    //getPipeId()
    //isEmpty()
    //setAsFixture()
    //connectFixture
    //setPipesAllowed
    //isPipeAllowed
    //connectPipeToCell
    //removeNonFixedConnectors
    //getCoordinate
    //isFixture
    //getEndpoint
    //getConnection
    //isConnected
    //isEndpoint
    //countWalls
    //countFixtureConnections
    //isBorderOpen
    //isHorizontalChannel
    //isVerticalChannel
    //operator==
    //removePipePossibility
}
