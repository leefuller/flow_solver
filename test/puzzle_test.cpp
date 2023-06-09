#include <iostream>

#include "../include/Puzzle.h"
#include <gtest/gtest.h>

TEST(puzzle_test, test_route)
{
    std::cout << "Test route" << std::endl;
    Route route = { createCoordinate(2,5) };
    Coordinate coord = createCoordinate(1, 1);
    EXPECT_FALSE(coordinateInRoute(coord, route));
    EXPECT_TRUE(coordinateInRoute(createCoordinate(2,5), route));
    route.push_back(createCoordinate(1,1));
    EXPECT_TRUE(coordinateInRoute(coord, route));
}

TEST(puzzle_test, test_distance_to_obstruction)
{
    std::cout << "Test distance to obstruction" << std::endl;
    const char * p = \
            {" = = = = = = = ,"  \
             "|. . .|. . . .|,"  \
             "|             |,"  \
             "|A B . . . B A|,"  \
             " = = = = = = = "   \
            };
    PuzzleDefinition def(p);
    std::shared_ptr<Puzzle> puzzle = def.generatePuzzle();
    for (int r = 0; r < puzzle->getNumRows(); ++r)
    {
        // Check against puzzle border
        for (int c = 0; c < puzzle->getNumCols(); ++c)
        {
            if (r == 0)
                EXPECT_TRUE(puzzle->gapToObstruction({r,c}, Direction::NORTH) == 0);
            if (r == puzzle->getNumRows() - 1)
                EXPECT_TRUE(puzzle->gapToObstruction({r,c}, Direction::SOUTH) == 0);
            if (c == 0)
                EXPECT_TRUE(puzzle->gapToObstruction({r,c}, Direction::WEST) == 0);
            if (c == puzzle->getNumCols() - 1)
                EXPECT_TRUE(puzzle->gapToObstruction({r,c}, Direction::EAST) == 0);
        }
    }
    EXPECT_TRUE(puzzle->gapToObstruction({0,0}, Direction::EAST) == 2);
    EXPECT_TRUE(puzzle->gapToObstruction({0,1}, Direction::EAST) == 1);
    EXPECT_TRUE(puzzle->gapToObstruction({0,2}, Direction::EAST) == 0);
    EXPECT_TRUE(puzzle->gapToObstruction({0,3}, Direction::EAST) == 3);
    EXPECT_TRUE(puzzle->gapToObstruction({0,4}, Direction::EAST) == 2);
    EXPECT_TRUE(puzzle->gapToObstruction({0,5}, Direction::EAST) == 1);
    EXPECT_TRUE(puzzle->gapToObstruction({0,6}, Direction::WEST) == 3);
    EXPECT_TRUE(puzzle->gapToObstruction({0,5}, Direction::WEST) == 2);
    EXPECT_TRUE(puzzle->gapToObstruction({0,4}, Direction::WEST) == 1);
    EXPECT_TRUE(puzzle->gapToObstruction({0,3}, Direction::WEST) == 0);
    EXPECT_TRUE(puzzle->gapToObstruction({0,2}, Direction::WEST) == 2);
    EXPECT_TRUE(puzzle->gapToObstruction({0,1}, Direction::WEST) == 1);
    EXPECT_TRUE(puzzle->gapToObstruction({0,0}, Direction::WEST) == 0);

    EXPECT_TRUE(puzzle->gapToObstruction({0,0}, Direction::SOUTH) == 0);
    EXPECT_TRUE(puzzle->gapToObstruction({0,1}, Direction::SOUTH) == 0);
    EXPECT_TRUE(puzzle->gapToObstruction({0,2}, Direction::SOUTH) == 1);
    EXPECT_TRUE(puzzle->gapToObstruction({0,3}, Direction::SOUTH) == 1);
    EXPECT_TRUE(puzzle->gapToObstruction({0,4}, Direction::SOUTH) == 1);
    EXPECT_TRUE(puzzle->gapToObstruction({0,5}, Direction::SOUTH) == 0);
    EXPECT_TRUE(puzzle->gapToObstruction({0,6}, Direction::SOUTH) == 0);

    EXPECT_TRUE(puzzle->gapToObstruction({1,0}, Direction::EAST) == 0);
    EXPECT_TRUE(puzzle->gapToObstruction({1,1}, Direction::EAST) == 3);
    EXPECT_TRUE(puzzle->gapToObstruction({1,2}, Direction::EAST) == 2);
    EXPECT_TRUE(puzzle->gapToObstruction({1,3}, Direction::EAST) == 1);
    EXPECT_TRUE(puzzle->gapToObstruction({1,4}, Direction::EAST) == 0);
    EXPECT_TRUE(puzzle->gapToObstruction({1,5}, Direction::EAST) == 0);
    EXPECT_TRUE(puzzle->gapToObstruction({1,6}, Direction::EAST) == 0);
    EXPECT_TRUE(puzzle->gapToObstruction({1,6}, Direction::WEST) == 0);
    EXPECT_TRUE(puzzle->gapToObstruction({1,5}, Direction::WEST) == 3);
    EXPECT_TRUE(puzzle->gapToObstruction({1,4}, Direction::WEST) == 2);
    EXPECT_TRUE(puzzle->gapToObstruction({1,3}, Direction::WEST) == 1);
    EXPECT_TRUE(puzzle->gapToObstruction({1,2}, Direction::WEST) == 0);
    EXPECT_TRUE(puzzle->gapToObstruction({1,1}, Direction::WEST) == 0);
    EXPECT_TRUE(puzzle->gapToObstruction({1,0}, Direction::WEST) == 0);

    EXPECT_TRUE(puzzle->gapToObstruction({1,0}, Direction::NORTH) == 1);
    EXPECT_TRUE(puzzle->gapToObstruction({1,1}, Direction::NORTH) == 1);
    EXPECT_TRUE(puzzle->gapToObstruction({1,2}, Direction::NORTH) == 1);
    EXPECT_TRUE(puzzle->gapToObstruction({1,3}, Direction::NORTH) == 1);
    EXPECT_TRUE(puzzle->gapToObstruction({1,4}, Direction::NORTH) == 1);
    EXPECT_TRUE(puzzle->gapToObstruction({1,5}, Direction::NORTH) == 1);
    EXPECT_TRUE(puzzle->gapToObstruction({1,5}, Direction::NORTH) == 1);
}

TEST(puzzle_test, test_generation)
{
    std::cout << "Test puzzle generation" << std::endl;
    const char * p = \
            {" = = = = = = = ,"  \
             "|. . . . . . .|,"  \
             "|             |,"  \
             "|A B . . . B A|,"  \
             " = = = = = = = "   \
            };
    PuzzleDefinition def(p);

    std::shared_ptr<Puzzle> puzzle = def.generatePuzzle();
    EXPECT_TRUE(puzzle->getNumRows() == 2);
    EXPECT_TRUE(puzzle->getNumCols() == 7);
    EXPECT_TRUE(puzzle->getNumPipes() == 2);

    // From top left
    EXPECT_TRUE(puzzle->isCoordinateChangeValid(createCoordinate(0,0), Direction::CENTRAL));
    EXPECT_TRUE(!puzzle->isCoordinateChangeValid(createCoordinate(0,0), Direction::WEST));
    EXPECT_TRUE(!puzzle->isCoordinateChangeValid(createCoordinate(0,0), Direction::NORTH_WEST));
    EXPECT_TRUE(!puzzle->isCoordinateChangeValid(createCoordinate(0,0), Direction::SOUTH_WEST));
    EXPECT_TRUE(puzzle->isCoordinateChangeValid(createCoordinate(0,0), Direction::EAST));
    EXPECT_TRUE(!puzzle->isCoordinateChangeValid(createCoordinate(0,0), Direction::NORTH_EAST));
    EXPECT_TRUE(puzzle->isCoordinateChangeValid(createCoordinate(0,0), Direction::SOUTH_EAST));
    EXPECT_TRUE(!puzzle->isCoordinateChangeValid(createCoordinate(0,0), Direction::NORTH));
    EXPECT_TRUE(puzzle->isCoordinateChangeValid(createCoordinate(0,0), Direction::SOUTH));

    // From top right
    EXPECT_TRUE(puzzle->isCoordinateChangeValid(createCoordinate(0,6), Direction::CENTRAL));
    EXPECT_TRUE(!puzzle->isCoordinateChangeValid(createCoordinate(0,6), Direction::EAST));
    EXPECT_TRUE(!puzzle->isCoordinateChangeValid(createCoordinate(0,6), Direction::SOUTH_EAST));
    EXPECT_TRUE(!puzzle->isCoordinateChangeValid(createCoordinate(0,6), Direction::NORTH_EAST));
    EXPECT_TRUE(!puzzle->isCoordinateChangeValid(createCoordinate(0,6), Direction::NORTH_WEST));
    EXPECT_TRUE(puzzle->isCoordinateChangeValid(createCoordinate(0,6), Direction::WEST));
    EXPECT_TRUE(puzzle->isCoordinateChangeValid(createCoordinate(0,6), Direction::SOUTH_WEST));
    EXPECT_TRUE(!puzzle->isCoordinateChangeValid(createCoordinate(0,0), Direction::NORTH));
    EXPECT_TRUE(puzzle->isCoordinateChangeValid(createCoordinate(0,0), Direction::SOUTH));

    // From bottom right
    EXPECT_TRUE(puzzle->isCoordinateChangeValid(createCoordinate(1,6), Direction::CENTRAL));
    EXPECT_TRUE(!puzzle->isCoordinateChangeValid(createCoordinate(1,6), Direction::EAST));
    EXPECT_TRUE(puzzle->isCoordinateChangeValid(createCoordinate(1,6), Direction::NORTH));
    EXPECT_TRUE(puzzle->isCoordinateChangeValid(createCoordinate(1,6), Direction::WEST));
    EXPECT_TRUE(puzzle->isCoordinateChangeValid(createCoordinate(1,6), Direction::NORTH_WEST));
    EXPECT_TRUE(!puzzle->isCoordinateChangeValid(createCoordinate(1,6), Direction::SOUTH_WEST));
    EXPECT_TRUE(!puzzle->isCoordinateChangeValid(createCoordinate(1,6), Direction::SOUTH));
    EXPECT_TRUE(!puzzle->isCoordinateChangeValid(createCoordinate(1,6), Direction::SOUTH_EAST));
    EXPECT_TRUE(!puzzle->isCoordinateChangeValid(createCoordinate(1,6), Direction::NORTH_EAST));

    std::shared_ptr<Cell> pCell = puzzle->getCellAtCoordinate(createCoordinate(0, 0));
    EXPECT_TRUE(pCell->getCoordinate() == createCoordinate(0, 0));
    EXPECT_TRUE(pCell->countFixtureConnections() == 0);
    EXPECT_TRUE(pCell->countWalls() == 2);
    EXPECT_TRUE(pCell->getBorder(Direction::NORTH) == CellBorder::WALL);
    EXPECT_TRUE(pCell->getBorder(Direction::SOUTH) == CellBorder::OPEN);
    EXPECT_TRUE(pCell->getBorder(Direction::WEST) == CellBorder::WALL);
    EXPECT_TRUE(pCell->getBorder(Direction::EAST) == CellBorder::OPEN);
    bool eq = (pCell->getBorders() == std::array<CellBorder, 4>({CellBorder::WALL,CellBorder::OPEN,CellBorder::WALL,CellBorder::OPEN}));
    EXPECT_TRUE(eq);
    EXPECT_TRUE(!pCell->isBorderOpen(Direction::NORTH));
    EXPECT_TRUE(pCell->isBorderOpen(Direction::SOUTH));
    EXPECT_TRUE(!pCell->isBorderOpen(Direction::WEST));
    EXPECT_TRUE(pCell->isBorderOpen(Direction::EAST));
    EXPECT_TRUE(pCell->getConnection(Direction::NORTH) == CellConnection::NO_CONNECTOR);
    EXPECT_TRUE(pCell->getConnection(Direction::SOUTH) == CellConnection::OPEN_CONNECTOR);
    EXPECT_TRUE(pCell->getConnection(Direction::WEST) == CellConnection::NO_CONNECTOR);
    EXPECT_TRUE(pCell->getConnection(Direction::EAST) == CellConnection::OPEN_CONNECTOR);
    eq = (pCell->getConnections() == std::array<CellConnection, 4>({CellConnection::NO_CONNECTOR,CellConnection::OPEN_CONNECTOR,CellConnection::NO_CONNECTOR,CellConnection::OPEN_CONNECTOR}));
    EXPECT_TRUE(eq);
    EXPECT_TRUE(!pCell->isConnected(Direction::NORTH));
    EXPECT_TRUE(!pCell->isConnected(Direction::SOUTH));
    EXPECT_TRUE(!pCell->isConnected(Direction::WEST));
    EXPECT_TRUE(!pCell->isConnected(Direction::EAST));
    EXPECT_TRUE(pCell->getEndpoint() == PipeEnd::NO_ENDPOINT);
    EXPECT_TRUE(pCell->getPipeId() == NO_PIPE_ID);
    EXPECT_TRUE(!pCell->isEndpoint());
    EXPECT_TRUE(!pCell->isFixture());
    EXPECT_TRUE(pCell->isEmpty());
    EXPECT_TRUE(!pCell->isHorizontalChannel());
    EXPECT_TRUE(!pCell->isVerticalChannel());

    EXPECT_TRUE(puzzle->isCellReachable(createCoordinate(0, 0)));
    EXPECT_TRUE(puzzle->isCellReachable(createCoordinate(0, 6)));
    EXPECT_TRUE(puzzle->isCellReachable(createCoordinate(1, 0)));
    EXPECT_TRUE(puzzle->isCellReachable(createCoordinate(1, 6)));
    EXPECT_TRUE(!puzzle->isCellReachable(createCoordinate(0, 7)));
    EXPECT_TRUE(!puzzle->isCellReachable(createCoordinate(2, 6)));

    /* TODO more puzzle tests
    std::shared_ptr<Cell> puzzle->getCellAdjacent (Coordinate coord, Direction direction);
    std::shared_ptr<const Cell> puzzle->getConstCellAdjacent (Coordinate coord, Direction direction);
    //std::set<const Cell *> puzzle->getAdjacentCells (Coordinate coord);
    std::array<std::shared_ptr<const Cell>, 9> puzzle->getAdjacentCells (Coordinate coord);
    std::shared_ptr<Cell> puzzle->getCellAdjacent (Coordinate coord, Adjacency direction);
    std::shared_ptr<const Cell> puzzle->getConstCellAdjacent (Coordinate coord, Adjacency direction);
    */

    Route route = { {0,3},{0,4} };
    puzzle->insertRoute ('X', route);
    puzzle->removeRoute ();
    // TODO verify above
    /* TODO more puzzle tests
    bool puzzle->traceRoute (PipeId idPipe, Route & route);
    void puzzle->traceRoutes (std::map<PipeId, Route> &);
    void puzzle->forEveryCell (std::function<void(std::shared_ptr<Cell>)> * f)
    void puzzle->forEachTraversalDirection (std::function<void(Direction d)> * f);
    */
    Coordinate coordEnd1 = puzzle->findPipeEnd('A', PipeEnd::PIPE_END_1);
    // The end point is assigned arbitrarily during puzzle validation
    EXPECT_TRUE(coordEnd1 == createCoordinate(1,0) || coordEnd1 == createCoordinate(1,6));
    Coordinate coordEnd2 = puzzle->findPipeEnd('A', PipeEnd::PIPE_END_2);
    EXPECT_TRUE(coordEnd2 == createCoordinate(1,0) || coordEnd2 == createCoordinate(1,6));
    EXPECT_TRUE(coordEnd1 != coordEnd2);

    EXPECT_TRUE(!puzzle->isEndpoint(createCoordinate(0,0)));
    EXPECT_TRUE(puzzle->isEndpoint(createCoordinate(1,0)));
    EXPECT_TRUE(puzzle->isEndpoint(createCoordinate(1,1)));
    EXPECT_TRUE(puzzle->isEndpoint(createCoordinate(1,6)));
    EXPECT_TRUE(puzzle->isEndpoint(createCoordinate(1,5)));

    std::set<Direction> directions = puzzle->getConnectedDirections (createCoordinate(0,0));
    EXPECT_TRUE(directions == std::set<Direction>({Direction::SOUTH, Direction::EAST}));
}
