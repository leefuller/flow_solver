#include <iostream>

#include "../include/Puzzle.h"
#include "test_helper.h"

int main ()
{
    bool result = true;
    //std::shared_ptr<LogScope> sl = logger.getScopedLogger("Test puzzle");
    Route route = { createCoordinate(2,5) };
    Coordinate coord = createCoordinate(1, 1);
    result = result && check(!coordinateInRoute(coord, route));
    result = result && check(coordinateInRoute(createCoordinate(2,5), route));
    route.push_back(createCoordinate(1,1));
    result = result && check(coordinateInRoute(coord, route));

    const char * p = \
            {" = = = = = = = ,"  \
             "|. . . . . . .|,"  \
             "|             |,"  \
             "|A B . . . B A|,"  \
             " = = = = = = = "   \
            };
    PuzzleDefinition def(p);
    std::shared_ptr<Puzzle> puzzle = def.generatePuzzle();
    result = result && check(puzzle->getNumRows() == 2);
    result = result && check(puzzle->getNumCols() == 7);
    result = result && check(puzzle->getNumPipes() == 2);

    // From top left
    result = result && check(puzzle->isCoordinateChangeValid(createCoordinate(0,0), Adjacency::ADJACENT_CENTRAL));
    result = result && check(!puzzle->isCoordinateChangeValid(createCoordinate(0,0), Adjacency::ADJACENT_WEST));
    result = result && check(!puzzle->isCoordinateChangeValid(createCoordinate(0,0), Adjacency::ADJACENT_NORTH_WEST));
    result = result && check(!puzzle->isCoordinateChangeValid(createCoordinate(0,0), Adjacency::ADJACENT_SOUTH_WEST));
    result = result && check(puzzle->isCoordinateChangeValid(createCoordinate(0,0), Adjacency::ADJACENT_EAST));
    result = result && check(!puzzle->isCoordinateChangeValid(createCoordinate(0,0), Adjacency::ADJACENT_NORTH_EAST));
    result = result && check(puzzle->isCoordinateChangeValid(createCoordinate(0,0), Adjacency::ADJACENT_SOUTH_EAST));
    result = result && check(!puzzle->isCoordinateChangeValid(createCoordinate(0,0), Adjacency::ADJACENT_NORTH));
    result = result && check(puzzle->isCoordinateChangeValid(createCoordinate(0,0), Adjacency::ADJACENT_SOUTH));

    // From top right
    result = result && check(puzzle->isCoordinateChangeValid(createCoordinate(0,6), Adjacency::ADJACENT_CENTRAL));
    result = result && check(!puzzle->isCoordinateChangeValid(createCoordinate(0,6), Adjacency::ADJACENT_EAST));
    result = result && check(!puzzle->isCoordinateChangeValid(createCoordinate(0,6), Adjacency::ADJACENT_SOUTH_EAST));
    result = result && check(!puzzle->isCoordinateChangeValid(createCoordinate(0,6), Adjacency::ADJACENT_NORTH_EAST));
    result = result && check(!puzzle->isCoordinateChangeValid(createCoordinate(0,6), Adjacency::ADJACENT_NORTH_WEST));
    result = result && check(puzzle->isCoordinateChangeValid(createCoordinate(0,6), Adjacency::ADJACENT_WEST));
    result = result && check(puzzle->isCoordinateChangeValid(createCoordinate(0,6), Adjacency::ADJACENT_SOUTH_WEST));
    result = result && check(!puzzle->isCoordinateChangeValid(createCoordinate(0,0), Adjacency::ADJACENT_NORTH));
    result = result && check(puzzle->isCoordinateChangeValid(createCoordinate(0,0), Adjacency::ADJACENT_SOUTH));

    // From bottom right
    result = result && check(puzzle->isCoordinateChangeValid(createCoordinate(1,6), Adjacency::ADJACENT_CENTRAL));
    result = result && check(!puzzle->isCoordinateChangeValid(createCoordinate(1,6), Adjacency::ADJACENT_EAST));
    result = result && check(puzzle->isCoordinateChangeValid(createCoordinate(1,6), Adjacency::ADJACENT_NORTH));
    result = result && check(puzzle->isCoordinateChangeValid(createCoordinate(1,6), Adjacency::ADJACENT_WEST));
    result = result && check(puzzle->isCoordinateChangeValid(createCoordinate(1,6), Adjacency::ADJACENT_NORTH_WEST));
    result = result && check(!puzzle->isCoordinateChangeValid(createCoordinate(1,6), Adjacency::ADJACENT_SOUTH_WEST));
    result = result && check(!puzzle->isCoordinateChangeValid(createCoordinate(1,6), Adjacency::ADJACENT_SOUTH));
    result = result && check(!puzzle->isCoordinateChangeValid(createCoordinate(1,6), Adjacency::ADJACENT_SOUTH_EAST));
    result = result && check(!puzzle->isCoordinateChangeValid(createCoordinate(1,6), Adjacency::ADJACENT_NORTH_EAST));

    std::shared_ptr<Cell> pCell = puzzle->getCellAtCoordinate(createCoordinate(0, 0));
    result = result && check(pCell->getCoordinate() == createCoordinate(0, 0));
    result = result && check(pCell->countFixtureConnections() == 0);
    result = result && check(pCell->countWalls() == 2);
    result = result && check(pCell->getBorder(Direction::UP) == CellBorder::WALL);
    result = result && check(pCell->getBorder(Direction::DOWN) == CellBorder::OPEN);
    result = result && check(pCell->getBorder(Direction::LEFT) == CellBorder::WALL);
    result = result && check(pCell->getBorder(Direction::RIGHT) == CellBorder::OPEN);
    result = result && check(pCell->getBorders() == std::array<CellBorder, 4>({CellBorder::WALL,CellBorder::OPEN,CellBorder::WALL,CellBorder::OPEN}));
    result = result && check(!pCell->isBorderOpen(Direction::UP));
    result = result && check(pCell->isBorderOpen(Direction::DOWN));
    result = result && check(!pCell->isBorderOpen(Direction::LEFT));
    result = result && check(pCell->isBorderOpen(Direction::RIGHT));
    result = result && check(pCell->getConnection(Direction::UP) == CellConnection::NO_CONNECTOR);
    result = result && check(pCell->getConnection(Direction::DOWN) == CellConnection::OPEN_CONNECTOR);
    result = result && check(pCell->getConnection(Direction::LEFT) == CellConnection::NO_CONNECTOR);
    result = result && check(pCell->getConnection(Direction::RIGHT) == CellConnection::OPEN_CONNECTOR);
    result = result && check(pCell->getConnections() == std::array<CellConnection, 4>({CellConnection::NO_CONNECTOR,CellConnection::OPEN_CONNECTOR,CellConnection::NO_CONNECTOR,CellConnection::OPEN_CONNECTOR}));
    result = result && check(!pCell->isConnected(Direction::UP));
    result = result && check(!pCell->isConnected(Direction::DOWN));
    result = result && check(!pCell->isConnected(Direction::LEFT));
    result = result && check(!pCell->isConnected(Direction::RIGHT));
    result = result && check(pCell->getEndpoint() == PipeEnd::NO_ENDPOINT);
    result = result && check(pCell->getPipeId() == NO_PIPE_ID);
    result = result && check(!pCell->isEndpoint());
    result = result && check(!pCell->isFixture());
    result = result && check(pCell->isEmpty());
    result = result && check(!pCell->isHorizontalChannel());
    result = result && check(!pCell->isVerticalChannel());
    //FIXME result = result && check(pCell->isPipeAllowed('A'));
    //FIXME result = result && check(pCell->isPipeAllowed('B'));
    //result = result && check(!pCell->isPipeAllowed('C'));

    /*
    // TODO return is the same cell as above
    std::shared_ptr<const Cell> pCellConst = puzzle->getConstCellAtCoordinate(createCoordinate(0, 0));
    */

    result = result && check(puzzle->isCellReachable(createCoordinate(0, 0)));
    result = result && check(puzzle->isCellReachable(createCoordinate(0, 6)));
    result = result && check(puzzle->isCellReachable(createCoordinate(1, 0)));
    result = result && check(puzzle->isCellReachable(createCoordinate(1, 6)));
    result = result && check(!puzzle->isCellReachable(createCoordinate(0, 7)));
    result = result && check(!puzzle->isCellReachable(createCoordinate(2, 6)));

    /* TODO
    std::shared_ptr<Cell> puzzle->getCellAdjacent (Coordinate coord, Direction direction);
    std::shared_ptr<const Cell> puzzle->getConstCellAdjacent (Coordinate coord, Direction direction);
    //std::set<const Cell *> puzzle->getAdjacentCells (Coordinate coord);
    std::array<std::shared_ptr<const Cell>, 9> puzzle->getAdjacentCells (Coordinate coord);
    std::shared_ptr<Cell> puzzle->getCellAdjacent (Coordinate coord, Adjacency direction);
    std::shared_ptr<const Cell> puzzle->getConstCellAdjacent (Coordinate coord, Adjacency direction);
    */

    std::set<Direction> directions = puzzle->getConnectedDirections (createCoordinate(0,0));
    result = result && check(directions == std::set<Direction>({Direction::DOWN, Direction::RIGHT}));
    /* TODO
    std::set<Direction> puzzle->getNowTraversableDirections (Coordinate coord, PipeId idPipe);
    bool puzzle->canNowTraverseDirectionFrom(Coordinate coord, Direction direction, PipeId idPipe);
    std::vector<std::shared_ptr<const Cell>> puzzle->getCellsUntilObstruction(Coordinate coord, Direction);
    */

    route = { {0,3},{0,4} };
    puzzle->insertRoute ('X', route);
    puzzle->removeRoute ();
    // TODO verify above
    /* TODO
    bool puzzle->traceRoute (PipeId idPipe, Route & route);
    void puzzle->traceRoutes (std::map<PipeId, Route> &);
    void puzzle->forEveryCell (std::function<void(std::shared_ptr<Cell>)> * f)
    //std::tuple<Coordinate, Coordinate> puzzle->getCoordinateRange ()
    void puzzle->forEachTraversalDirection (std::function<void(Direction d)> * f);
    */
    Coordinate coordEnd1 = puzzle->findPipeEnd('A', PipeEnd::PIPE_END_1);
    // The end point is assigned arbitrarily during puzzle validation
    result = result && check(coordEnd1 == createCoordinate(1,0) || coordEnd1 == createCoordinate(1,6));
    Coordinate coordEnd2 = puzzle->findPipeEnd('A', PipeEnd::PIPE_END_2);
    result = result && check(coordEnd2 == createCoordinate(1,0) || coordEnd2 == createCoordinate(1,6));
    result = result && check(coordEnd1 != coordEnd2);

    result = result && check(!puzzle->isEndpoint(createCoordinate(0,0)));
    result = result && check(puzzle->isEndpoint(createCoordinate(1,0)));
    result = result && check(puzzle->isEndpoint(createCoordinate(1,1)));
    result = result && check(puzzle->isEndpoint(createCoordinate(1,6)));
    result = result && check(puzzle->isEndpoint(createCoordinate(1,5)));

    /* TODO
    bool puzzle->isHorizontalChannel (Coordinate coord);
    bool puzzle->isVerticalChannel (Coordinate coord);
    bool puzzle->isAdjacentToChannelOpening (Coordinate coord);
    */

    return result ? 0 : 1;
}
