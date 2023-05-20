#include "../include/Direction.h"
#include "test_helper.h"

#include <iostream>

bool testCoordinateChange ()
{
    bool result = true;
    //std::shared_ptr<LogScope> sl = logger.getScopedLogger("Test coordinate change");
    Coordinate coord = createCoordinate(2, 6);
    result = result && check(coord[0] == 2);
    result = result && check(coord[1] == 6);
    result = result && check(coordinateChange(coord, Direction::UP));
    result = result && check(coord[0] == 1);
    result = result && check(coord[1] == 6);
    result = result && check(coordinateChange(coord, Direction::DOWN));
    result = result && check(coord[0] == 2);
    result = result && check(coord[1] == 6);
    result = result && check(coordinateChange(coord, Direction::LEFT));
    result = result && check(coord[0] == 2);
    result = result && check(coord[1] == 5);
    result = result && check(coordinateChange(coord, Direction::RIGHT));
    result = result && check(coord[0] == 2);
    result = result && check(coord[1] == 6);

    result = result && check(coordinateChange (coord, Adjacency::ADJACENT_NORTH));
    result = result && check(coord[0] == 1);
    result = result && check(coord[1] == 6);
    result = result && check(coordinateChange (coord, Adjacency::ADJACENT_WEST));
    result = result && check(coord[0] == 1);
    result = result && check(coord[1] == 5);
    result = result && check(coordinateChange (coord, Adjacency::ADJACENT_SOUTH));
    result = result && check(coord[0] == 2);
    result = result && check(coord[1] == 5);
    result = result && check(coordinateChange (coord, Adjacency::ADJACENT_NORTH_EAST));
    result = result && check(coord[0] == 1);
    result = result && check(coord[1] == 6);
    result = result && check(coordinateChange (coord, Adjacency::ADJACENT_NORTH_WEST));
    result = result && check(coord[0] == 0);
    result = result && check(coord[1] == 5);
    result = result && check(coordinateChange (coord, Adjacency::ADJACENT_EAST));
    result = result && check(coord[0] == 0);
    result = result && check(coord[1] == 6);
    result = result && check(coordinateChange (coord, Adjacency::ADJACENT_SOUTH_EAST));
    result = result && check(coord[0] == 1);
    result = result && check(coord[1] == 7);
    result = result && check(coordinateChange (coord, Adjacency::ADJACENT_SOUTH_WEST));
    result = result && check(coord[0] == 2);
    result = result && check(coord[1] == 6);

    coord = createCoordinate(1, 1);
    result = result && check(coordinateChange(coord, Direction::LEFT));
    result = result && check(coord[0] == 1);
    result = result && check(coord[1] == 0);
    result = result && check(!coordinateChange(coord, Direction::LEFT));
    result = result && check(coord[0] == 1);
    result = result && check(coord[1] == 0);
    result = result && check(coordinateChange(coord, Direction::UP));
    result = result && check(coord[0] == 0);
    result = result && check(coord[1] == 0);
    result = result && check(!coordinateChange(coord, Direction::UP));
    result = result && check(coord[0] == 0);
    result = result && check(coord[1] == 0);
    return result;
}

bool testDirectionAsString ()
{
    //std::shared_ptr<LogScope> sl = logger.getScopedLogger("Test direction");
    bool result = true;

    result = result && check("UP as string", std::string(asString(Direction::UP)) == "up");
    result = result && check("DOWN as string", std::string(asString(Direction::DOWN)) == "down");
    result = result && check("LEFT as string", std::string(asString(Direction::LEFT)) == "left");
    result = result && check("RIGHT as string", std::string(asString(Direction::RIGHT)) == "right");
    result = result && check("NONE as string", std::string(asString(Direction::NONE)) == "none");
    return result;
}

bool testDirection ()
{
    bool result = true;

    result = result && check(opposite(Direction::UP) == Direction::DOWN);
    result = result && check(opposite(Direction::DOWN) == Direction::UP);
    result = result && check(opposite(Direction::LEFT) == Direction::RIGHT);
    result = result && check(opposite(Direction::RIGHT) == Direction::LEFT);

    result = result && check(opposite(Adjacency::ADJACENT_NORTH_WEST) == Adjacency::ADJACENT_SOUTH_EAST);
    result = result && check(opposite(Adjacency::ADJACENT_NORTH) == Adjacency::ADJACENT_SOUTH);
    result = result && check(opposite(Adjacency::ADJACENT_NORTH_EAST) == Adjacency::ADJACENT_SOUTH_WEST);
    result = result && check(opposite(Adjacency::ADJACENT_WEST) == Adjacency::ADJACENT_EAST);
    result = result && check(opposite(Adjacency::ADJACENT_CENTRAL) == Adjacency::ADJACENT_CENTRAL);
    result = result && check(opposite(Adjacency::ADJACENT_EAST) == Adjacency::ADJACENT_WEST);
    result = result && check(opposite(Adjacency::ADJACENT_SOUTH_WEST) == Adjacency::ADJACENT_NORTH_EAST);
    result = result && check(opposite(Adjacency::ADJACENT_SOUTH) == Adjacency::ADJACENT_NORTH);
    result = result && check(opposite(Adjacency::ADJACENT_SOUTH_EAST) == Adjacency::ADJACENT_NORTH_WEST);

    result = result && check(isVertical(Direction::UP));
    result = result && check(isVertical(Direction::DOWN));
    result = result && check(!isVertical(Direction::LEFT));
    result = result && check(!isVertical(Direction::RIGHT));
    result = result && check(!isVertical(Direction::NONE));

    result = result && check(allTraversalDirectionsSet.find(Direction::UP) != allTraversalDirectionsSet.end());
    result = result && check(allTraversalDirectionsSet.find(Direction::DOWN) != allTraversalDirectionsSet.end());
    result = result && check(allTraversalDirectionsSet.find(Direction::LEFT) != allTraversalDirectionsSet.end());
    result = result && check(allTraversalDirectionsSet.find(Direction::RIGHT) != allTraversalDirectionsSet.end());

    result = result && check(std::find(allTraversalDirections.begin(), allTraversalDirections.end(), Direction::UP) != allTraversalDirections.end());
    result = result && check(std::find(allTraversalDirections.begin(), allTraversalDirections.end(), Direction::DOWN) != allTraversalDirections.end());
    result = result && check(std::find(allTraversalDirections.begin(), allTraversalDirections.end(), Direction::LEFT) != allTraversalDirections.end());
    result = result && check(std::find(allTraversalDirections.begin(), allTraversalDirections.end(), Direction::RIGHT) != allTraversalDirections.end());

    // TODO
    //forEachTraversalDirection([](){  });
    return result;
}

bool testAreAdjacent ()
{
    bool result = true;
    // Check at 0,0 where only DOWN and RIGHT are possible
    result = result && check(areAdjacent({0,0}, {0,0}) == Direction::NONE);
    result = result && check(areAdjacent({0,0}, {1,0}) == Direction::DOWN);
    result = result && check(areAdjacent({0,0}, {0,1}) == Direction::RIGHT);
    result = result && check(areAdjacent({0,0}, {2,0}) == Direction::NONE);
    result = result && check(areAdjacent({0,0}, {0,2}) == Direction::NONE);
    result = result && check(areAdjacent({0,0}, {1,1}) == Direction::NONE); // diagonal is not adjacent

    // check from 1,1
    result = result && check(areAdjacent({1,1}, {1,1}) == Direction::NONE);
    result = result && check(areAdjacent({1,1}, {2,1}) == Direction::DOWN);
    result = result && check(areAdjacent({1,1}, {1,2}) == Direction::RIGHT);
    result = result && check(areAdjacent({1,1}, {1,0}) == Direction::LEFT);
    result = result && check(areAdjacent({1,1}, {0,1}) == Direction::UP);
    result = result && check(areAdjacent({1,1}, {3,1}) == Direction::NONE);
    result = result && check(areAdjacent({1,1}, {1,3}) == Direction::NONE);
    result = result && check(areAdjacent({1,2}, {1,0}) == Direction::NONE);
    result = result && check(areAdjacent({2,1}, {0,1}) == Direction::NONE);
    result = result && check(areAdjacent({1,1}, {0,0}) == Direction::NONE); // diagonal is not adjacent
    result = result && check(areAdjacent({1,1}, {2,2}) == Direction::NONE); // diagonal is not adjacent
    return result;
}

int main ()
{
    bool result = true;
    std::cout << "Test direction" << std::endl;
    result = result && testCoordinateChange();
    result = result && testDirectionAsString();
    result = result && testDirection();
    result = result && testAreAdjacent();
    return result ? 0 : 1;
}
