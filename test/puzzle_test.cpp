#include <iostream>

#include "../include/Puzzle.h"
#include "test_helper.h"

bool testRoute ()
{
    std::cout << "Test route" << std::endl;
    bool result = true;
    Route route = { createCoordinate(2,5) };
    Coordinate coord = createCoordinate(1, 1);
    result = result && check(!coordinateInRoute(coord, route));
    result = result && check(coordinateInRoute(createCoordinate(2,5), route));
    route.push_back(createCoordinate(1,1));
    result = result && check(coordinateInRoute(coord, route));
    return result;
}

bool testdistanceToObstruction ()
{
    std::cout << "Test distance to obstruction" << std::endl;
    bool result = true;
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
                result = result && check(puzzle->gapToObstruction({r,c}, Direction::NORTH) == 0);
            if (r == puzzle->getNumRows() - 1)
                result = result && check(puzzle->gapToObstruction({r,c}, Direction::SOUTH) == 0);
            if (c == 0)
                result = result && check(puzzle->gapToObstruction({r,c}, Direction::WEST) == 0);
            if (c == puzzle->getNumCols() - 1)
                result = result && check(puzzle->gapToObstruction({r,c}, Direction::EAST) == 0);
        }
    }
    result = result && check(puzzle->gapToObstruction({0,0}, Direction::EAST) == 2);
    result = result && check(puzzle->gapToObstruction({0,1}, Direction::EAST) == 1);
    result = result && check(puzzle->gapToObstruction({0,2}, Direction::EAST) == 0);
    result = result && check(puzzle->gapToObstruction({0,3}, Direction::EAST) == 3);
    result = result && check(puzzle->gapToObstruction({0,4}, Direction::EAST) == 2);
    result = result && check(puzzle->gapToObstruction({0,5}, Direction::EAST) == 1);
    result = result && check(puzzle->gapToObstruction({0,6}, Direction::WEST) == 3);
    result = result && check(puzzle->gapToObstruction({0,5}, Direction::WEST) == 2);
    result = result && check(puzzle->gapToObstruction({0,4}, Direction::WEST) == 1);
    result = result && check(puzzle->gapToObstruction({0,3}, Direction::WEST) == 0);
    result = result && check(puzzle->gapToObstruction({0,2}, Direction::WEST) == 2);
    result = result && check(puzzle->gapToObstruction({0,1}, Direction::WEST) == 1);
    result = result && check(puzzle->gapToObstruction({0,0}, Direction::WEST) == 0);

    result = result && check(puzzle->gapToObstruction({0,0}, Direction::SOUTH) == 0);
    result = result && check(puzzle->gapToObstruction({0,1}, Direction::SOUTH) == 0);
    result = result && check(puzzle->gapToObstruction({0,2}, Direction::SOUTH) == 1);
    result = result && check(puzzle->gapToObstruction({0,3}, Direction::SOUTH) == 1);
    result = result && check(puzzle->gapToObstruction({0,4}, Direction::SOUTH) == 1);
    result = result && check(puzzle->gapToObstruction({0,5}, Direction::SOUTH) == 0);
    result = result && check(puzzle->gapToObstruction({0,6}, Direction::SOUTH) == 0);

    result = result && check(puzzle->gapToObstruction({1,0}, Direction::EAST) == 0);
    result = result && check(puzzle->gapToObstruction({1,1}, Direction::EAST) == 3);
    result = result && check(puzzle->gapToObstruction({1,2}, Direction::EAST) == 2);
    result = result && check(puzzle->gapToObstruction({1,3}, Direction::EAST) == 1);
    result = result && check(puzzle->gapToObstruction({1,4}, Direction::EAST) == 0);
    result = result && check(puzzle->gapToObstruction({1,5}, Direction::EAST) == 0);
    result = result && check(puzzle->gapToObstruction({1,6}, Direction::EAST) == 0);
    result = result && check(puzzle->gapToObstruction({1,6}, Direction::WEST) == 0);
    result = result && check(puzzle->gapToObstruction({1,5}, Direction::WEST) == 3);
    result = result && check(puzzle->gapToObstruction({1,4}, Direction::WEST) == 2);
    result = result && check(puzzle->gapToObstruction({1,3}, Direction::WEST) == 1);
    result = result && check(puzzle->gapToObstruction({1,2}, Direction::WEST) == 0);
    result = result && check(puzzle->gapToObstruction({1,1}, Direction::WEST) == 0);
    result = result && check(puzzle->gapToObstruction({1,0}, Direction::WEST) == 0);

    result = result && check(puzzle->gapToObstruction({1,0}, Direction::NORTH) == 1);
    result = result && check(puzzle->gapToObstruction({1,1}, Direction::NORTH) == 1);
    result = result && check(puzzle->gapToObstruction({1,2}, Direction::NORTH) == 1);
    result = result && check(puzzle->gapToObstruction({1,3}, Direction::NORTH) == 1);
    result = result && check(puzzle->gapToObstruction({1,4}, Direction::NORTH) == 1);
    result = result && check(puzzle->gapToObstruction({1,5}, Direction::NORTH) == 1);
    result = result && check(puzzle->gapToObstruction({1,5}, Direction::NORTH) == 1);

    return result;
}

bool testPuzzleGeneration ()
{
    std::cout << "Test puzzle generation" << std::endl;
    bool result = true;
    const char * p = \
            {" = = = = = = = ,"  \
             "|. . . . . . .|,"  \
             "|             |,"  \
             "|A B . . . B A|,"  \
             " = = = = = = = "   \
            };
    PuzzleDefinition def(p);
#if 0
    std::shared_ptr<Puzzle> puzzle = def.generatePuzzle();
    result = result && check("generated puzzle has correct number of rows", puzzle->getNumRows() == 2);
    result = result && check("generated puzzle has correct number of columns", puzzle->getNumCols() == 7);
    result = result && check("generated puzzle has correct number of pipes", puzzle->getNumPipes() == 2);

    // From top left
    result = result && check("from top left, central", puzzle->isCoordinateChangeValid(createCoordinate(0,0), Direction::CENTRAL));
    result = result && check("from top left, cannot go west", !puzzle->isCoordinateChangeValid(createCoordinate(0,0), Direction::WEST));
    result = result && check("from top left, cannot go north west", !puzzle->isCoordinateChangeValid(createCoordinate(0,0), Direction::NORTH_WEST));
    result = result && check("from top left, cannot go south west", !puzzle->isCoordinateChangeValid(createCoordinate(0,0), Direction::SOUTH_WEST));
    result = result && check("from top left, can go east", puzzle->isCoordinateChangeValid(createCoordinate(0,0), Direction::EAST));
    result = result && check("from top left, cannot go north east", !puzzle->isCoordinateChangeValid(createCoordinate(0,0), Direction::NORTH_EAST));
    result = result && check("from top left, can go south east", puzzle->isCoordinateChangeValid(createCoordinate(0,0), Direction::SOUTH_EAST));
    result = result && check("from top left, cannot go north", !puzzle->isCoordinateChangeValid(createCoordinate(0,0), Direction::NORTH));
    result = result && check("from top left, can go south", puzzle->isCoordinateChangeValid(createCoordinate(0,0), Direction::SOUTH));

    // From top right
    result = result && check("from top right, central", puzzle->isCoordinateChangeValid(createCoordinate(0,6), Direction::CENTRAL));
    result = result && check("from top right, cannot go east", !puzzle->isCoordinateChangeValid(createCoordinate(0,6), Direction::EAST));
    result = result && check("from top right, cannot go south east", !puzzle->isCoordinateChangeValid(createCoordinate(0,6), Direction::SOUTH_EAST));
    result = result && check("from top right, cannot go north east", !puzzle->isCoordinateChangeValid(createCoordinate(0,6), Direction::NORTH_EAST));
    result = result && check("from top right, cannot go north west", !puzzle->isCoordinateChangeValid(createCoordinate(0,6), Direction::NORTH_WEST));
    result = result && check("from top right, can go west", puzzle->isCoordinateChangeValid(createCoordinate(0,6), Direction::WEST));
    result = result && check("from top right, can go south west", puzzle->isCoordinateChangeValid(createCoordinate(0,6), Direction::SOUTH_WEST));
    result = result && check("from top right, cannot go north", !puzzle->isCoordinateChangeValid(createCoordinate(0,0), Direction::NORTH));
    result = result && check("from top right, can go south", puzzle->isCoordinateChangeValid(createCoordinate(0,0), Direction::SOUTH));

    // From bottom right
    result = result && check("from bottom right, central", puzzle->isCoordinateChangeValid(createCoordinate(1,6), Direction::CENTRAL));
    result = result && check("from bottom right, cannot go east", !puzzle->isCoordinateChangeValid(createCoordinate(1,6), Direction::EAST));
    result = result && check("from bottom right, can go north", puzzle->isCoordinateChangeValid(createCoordinate(1,6), Direction::NORTH));
    result = result && check("from bottom right, can go west", puzzle->isCoordinateChangeValid(createCoordinate(1,6), Direction::WEST));
    result = result && check("from bottom right, can go north west", puzzle->isCoordinateChangeValid(createCoordinate(1,6), Direction::NORTH_WEST));
    result = result && check("from bottom right, cannot go south west", !puzzle->isCoordinateChangeValid(createCoordinate(1,6), Direction::SOUTH_WEST));
    result = result && check("from bottom right, cannot go south", !puzzle->isCoordinateChangeValid(createCoordinate(1,6), Direction::SOUTH));
    result = result && check("from bottom right, cannot go south east", !puzzle->isCoordinateChangeValid(createCoordinate(1,6), Direction::SOUTH_EAST));
    result = result && check("from bottom right, cannot go north east", !puzzle->isCoordinateChangeValid(createCoordinate(1,6), Direction::NORTH_EAST));

    std::shared_ptr<Cell> pCell = puzzle->getCellAtCoordinate(createCoordinate(0, 0));
    result = result && check("Test cell getCoordinate()", pCell->getCoordinate() == createCoordinate(0, 0));
    result = result && check("Test cell no connections counted", pCell->countFixtureConnections() == 0);
    result = result && check("Test cell {0,0} has 2 walls", pCell->countWalls() == 2);
    result = result && check("Test cell {0,0} has wall north", pCell->getBorder(Direction::NORTH) == CellBorder::WALL);
    result = result && check("Test cell {0,0} has no wall south", pCell->getBorder(Direction::SOUTH) == CellBorder::OPEN);
    result = result && check("Test cell {0,0} has wall west", pCell->getBorder(Direction::WEST) == CellBorder::WALL);
    result = result && check("Test cell {0,0} has no wall east", pCell->getBorder(Direction::EAST) == CellBorder::OPEN);
    result = result && check("Test cell getBorders()", pCell->getBorders() == std::array<CellBorder, 4>({CellBorder::WALL,CellBorder::OPEN,CellBorder::WALL,CellBorder::OPEN}));
    result = result && check("Test cell isBorderOpen(NORTH)", !pCell->isBorderOpen(Direction::NORTH));
    result = result && check("Test cell isBorderOpen(SOUTH)", pCell->isBorderOpen(Direction::SOUTH));
    result = result && check("Test cell isBorderOpen(WEST)", !pCell->isBorderOpen(Direction::WEST));
    result = result && check("Test cell isBorderOpen(EAST)", pCell->isBorderOpen(Direction::EAST));
    result = result && check("Test cell {0,0} no connection north", pCell->getConnection(Direction::NORTH) == CellConnection::NO_CONNECTOR);
    result = result && check("Test cell {0,0} connects south", pCell->getConnection(Direction::SOUTH) == CellConnection::OPEN_CONNECTOR);
    result = result && check("Test cell {0,0} no connection west", pCell->getConnection(Direction::WEST) == CellConnection::NO_CONNECTOR);
    result = result && check("Test cell {0,0} connects east", pCell->getConnection(Direction::EAST) == CellConnection::OPEN_CONNECTOR);
    result = result && check("Test cell getConnections()", pCell->getConnections() == std::array<CellConnection, 4>({CellConnection::NO_CONNECTOR,CellConnection::OPEN_CONNECTOR,CellConnection::NO_CONNECTOR,CellConnection::OPEN_CONNECTOR}));
    result = result && check("Test cell isConnected", !pCell->isConnected(Direction::NORTH));
    result = result && check("Test cell isConnected", !pCell->isConnected(Direction::SOUTH));
    result = result && check("Test cell isConnected", !pCell->isConnected(Direction::WEST));
    result = result && check("Test cell isConnected", !pCell->isConnected(Direction::EAST));
    result = result && check("Test cell {0,0} endpoint type = none", pCell->getEndpoint() == PipeEnd::NO_ENDPOINT);
    result = result && check("Test cell {0,0} has no pipe", pCell->getPipeId() == NO_PIPE_ID);
    result = result && check("Test cell {0,0} not an endpoint", !pCell->isEndpoint());
    result = result && check("Test cell {0,0} not a fixture", !pCell->isFixture());
    result = result && check("Test cell {0,0} empty", pCell->isEmpty());
    result = result && check("Test cell {0,0} not a horizontal channel", !pCell->isHorizontalChannel());
    result = result && check("Test cell {0,0} not a vertical channel", !pCell->isVerticalChannel());

    result = result && check("Test cell {0,0} reachable", puzzle->isCellReachable(createCoordinate(0, 0)));
    result = result && check("Test cell {0,6} reachable", puzzle->isCellReachable(createCoordinate(0, 6)));
    result = result && check("Test cell {1,0} reachable", puzzle->isCellReachable(createCoordinate(1, 0)));
    result = result && check("Test cell {1,6} reachable", puzzle->isCellReachable(createCoordinate(1, 6)));
    result = result && check("Test cell {0,7} not reachable", !puzzle->isCellReachable(createCoordinate(0, 7)));
    result = result && check("Test cell {2,6} not reachable", !puzzle->isCellReachable(createCoordinate(2, 6)));

    /* TODO
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
    /* TODO
    bool puzzle->traceRoute (PipeId idPipe, Route & route);
    void puzzle->traceRoutes (std::map<PipeId, Route> &);
    void puzzle->forEveryCell (std::function<void(std::shared_ptr<Cell>)> * f)
    void puzzle->forEachTraversalDirection (std::function<void(Direction d)> * f);
    */
    Coordinate coordEnd1 = puzzle->findPipeEnd('A', PipeEnd::PIPE_END_1);
    // The end point is assigned arbitrarily during puzzle validation
    result = result && check("find endpoint 1", coordEnd1 == createCoordinate(1,0) || coordEnd1 == createCoordinate(1,6));
    Coordinate coordEnd2 = puzzle->findPipeEnd('A', PipeEnd::PIPE_END_2);
    result = result && check("find endpoint 2", coordEnd2 == createCoordinate(1,0) || coordEnd2 == createCoordinate(1,6));
    result = result && check("check endpoints differ", coordEnd1 != coordEnd2);

    result = result && check("check {0,0} not an endpoint", !puzzle->isEndpoint(createCoordinate(0,0)));
    result = result && check("check {1,0} is an endpoint", puzzle->isEndpoint(createCoordinate(1,0)));
    result = result && check("check {1,1} is an endpoint", puzzle->isEndpoint(createCoordinate(1,1)));
    result = result && check("check {1,6} is an endpoint", puzzle->isEndpoint(createCoordinate(1,6)));
    result = result && check("check {1,5} is an endpoint", puzzle->isEndpoint(createCoordinate(1,5)));

    std::set<Direction> directions = puzzle->getConnectedDirections (createCoordinate(0,0));
    result = result && check("check {0,0} connectedDirections", directions == std::set<Direction>({Direction::SOUTH, Direction::EAST}));
#endif

    return result;
}

int main ()
{
    bool result = true;
    try
    {
        result = result && testRoute();
        result = result && testPuzzleGeneration();
        result = result && testdistanceToObstruction();
    }
    catch (std::exception & ex)
    {
        std::cerr << "Exception: " << ex.what() << std::endl;
        return 1;
    }
    return result ? 0 : 1;
}
