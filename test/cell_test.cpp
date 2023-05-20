#include <iostream>

#include "../include/Cell.h"
#include "test_helper.h"

int main ()
{
    bool result = true;
    //std::shared_ptr<LogScope> sl = logger.getScopedLogger("Test cell");

    CellBorder edge = CellBorder::OPEN;
    check(toggleBorder(edge) == CellBorder::WALL);
    check(edge == CellBorder::OPEN);
    edge = CellBorder::WALL;
    result = result && check(toggleBorder(edge) == CellBorder::OPEN);
    result = result && check(edge == CellBorder::WALL);

    //std::array<CellBorder, 4> border = { CellBorder::OPEN, CellBorder::OPEN, CellBorder::OPEN, CellBorder::OPEN};

    result = result && check(getOppositeBorder(
            std::array<CellBorder, 4>({CellBorder::OPEN, CellBorder::OPEN, CellBorder::OPEN, CellBorder::OPEN})) ==
            std::array<CellBorder, 4>({CellBorder::WALL, CellBorder::WALL, CellBorder::WALL, CellBorder::WALL}));
    result = result && check(getOppositeBorder(
            std::array<CellBorder, 4>({CellBorder::WALL, CellBorder::OPEN, CellBorder::OPEN, CellBorder::OPEN})) ==
            std::array<CellBorder, 4>({CellBorder::OPEN, CellBorder::WALL, CellBorder::WALL, CellBorder::WALL}));
    result = result && check(getOppositeBorder(
            std::array<CellBorder, 4>({CellBorder::OPEN, CellBorder::WALL, CellBorder::OPEN, CellBorder::OPEN})) ==
            std::array<CellBorder, 4>({CellBorder::WALL, CellBorder::OPEN, CellBorder::WALL, CellBorder::WALL}));
    result = result && check(getOppositeBorder(
            std::array<CellBorder, 4>({CellBorder::OPEN, CellBorder::OPEN, CellBorder::WALL, CellBorder::OPEN})) ==
            std::array<CellBorder, 4>({CellBorder::WALL, CellBorder::WALL, CellBorder::OPEN, CellBorder::WALL}));
    result = result && check(getOppositeBorder(
            std::array<CellBorder, 4>({CellBorder::OPEN, CellBorder::OPEN, CellBorder::OPEN, CellBorder::WALL})) ==
            std::array<CellBorder, 4>({CellBorder::WALL, CellBorder::WALL, CellBorder::WALL, CellBorder::OPEN}));

    //Cell cell; // private
    // TODO
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
    return result ? 0 : 1;
}
