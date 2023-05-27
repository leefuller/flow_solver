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
    result = result && check("Coordinate change north", coordinateChange(coord, Direction::NORTH));
    result = result && check("North reduces row index", coord[0] == 1);
    result = result && check(coord[1] == 6);
    result = result && check("Coordinate change south", coordinateChange(coord, Direction::SOUTH));
    result = result && check("South increases row index", coord[0] == 2);
    result = result && check(coord[1] == 6);
    result = result && check("Coordinate change west", coordinateChange(coord, Direction::WEST));
    result = result && check(coord[0] == 2);
    result = result && check("West reduces column index", coord[1] == 5);
    result = result && check("Coordinate change east", coordinateChange(coord, Direction::EAST));
    result = result && check(coord[0] == 2);
    result = result && check("East increases column index", coord[1] == 6);

    result = result && check("Coordinate change north", coordinateChange(coord, Direction::NORTH));
    result = result && check(coord[0] == 1);
    result = result && check(coord[1] == 6);
    result = result && check("Coordinate change west", coordinateChange(coord, Direction::WEST));
    result = result && check(coord[0] == 1);
    result = result && check(coord[1] == 5);
    result = result && check("Coordinate change south", coordinateChange(coord, Direction::SOUTH));
    result = result && check(coord[0] == 2);
    result = result && check(coord[1] == 5);
    result = result && check("Coordinate change north east", coordinateChange(coord, Direction::NORTH_EAST));
    result = result && check(coord[0] == 1);
    result = result && check(coord[1] == 6);
    result = result && check("Coordinate change north west", coordinateChange(coord, Direction::NORTH_WEST));
    result = result && check(coord[0] == 0);
    result = result && check(coord[1] == 5);
    result = result && check("Coordinate change east", coordinateChange(coord, Direction::EAST));
    result = result && check(coord[0] == 0);
    result = result && check(coord[1] == 6);
    result = result && check("Coordinate change south east", coordinateChange(coord, Direction::SOUTH_EAST));
    result = result && check(coord[0] == 1);
    result = result && check(coord[1] == 7);
    result = result && check("Coordinate change south west", coordinateChange(coord, Direction::SOUTH_WEST));
    result = result && check(coord[0] == 2);
    result = result && check(coord[1] == 6);

    coord = createCoordinate(1, 1);
    result = result && check("Can go west from {1,1}", coordinateChange(coord, Direction::WEST));
    result = result && check(coord[0] == 1);
    result = result && check(coord[1] == 0);
    result = result && check("Cannot go west from {1,0}", !coordinateChange(coord, Direction::WEST));
    result = result && check(coord[0] == 1);
    result = result && check(coord[1] == 0);
    result = result && check("Can go north from {1,0}", coordinateChange(coord, Direction::NORTH));
    result = result && check(coord[0] == 0);
    result = result && check(coord[1] == 0);
    result = result && check("Cannot go north from {0,0}", !coordinateChange(coord, Direction::NORTH));
    result = result && check(coord[0] == 0);
    result = result && check(coord[1] == 0);
    return result;
}

bool testDirectionAsString ()
{
    //std::shared_ptr<LogScope> sl = logger.getScopedLogger("Test direction");
    bool result = true;

    result = result && check("NORTH as string", std::string(asString(Direction::NORTH)) == "north");
    result = result && check("SOUTH as string", std::string(asString(Direction::SOUTH)) == "south");
    result = result && check("WEST as string", std::string(asString(Direction::WEST)) == "west");
    result = result && check("EAST as string", std::string(asString(Direction::EAST)) == "east");
    result = result && check("NONE as string", std::string(asString(Direction::NONE)) == "none");
    return result;
}

bool testDirection ()
{
    bool result = true;

    result = result && check(opposite(Direction::NORTH) == Direction::SOUTH);
    result = result && check(opposite(Direction::SOUTH) == Direction::NORTH);
    result = result && check(opposite(Direction::WEST) == Direction::EAST);
    result = result && check(opposite(Direction::EAST) == Direction::WEST);

    result = result && check(opposite(Direction::NORTH_WEST) == Direction::SOUTH_EAST);
    result = result && check(opposite(Direction::NORTH) == Direction::SOUTH);
    result = result && check(opposite(Direction::NORTH_EAST) == Direction::SOUTH_WEST);
    result = result && check(opposite(Direction::WEST) == Direction::EAST);
    result = result && check(opposite(Direction::CENTRAL) == Direction::CENTRAL);
    result = result && check(opposite(Direction::EAST) == Direction::WEST);
    result = result && check(opposite(Direction::SOUTH_WEST) == Direction::NORTH_EAST);
    result = result && check(opposite(Direction::SOUTH) == Direction::NORTH);
    result = result && check(opposite(Direction::SOUTH_EAST) == Direction::NORTH_WEST);
    result = result && check(opposite(Direction::NONE) == Direction::NONE);

    result = result && check(isVertical(Direction::NORTH));
    result = result && check(isVertical(Direction::SOUTH));
    result = result && check(!isVertical(Direction::WEST));
    result = result && check(!isVertical(Direction::EAST));
    result = result && check(!isVertical(Direction::NORTH_EAST));
    result = result && check(!isVertical(Direction::NORTH_WEST));
    result = result && check(!isVertical(Direction::SOUTH_EAST));
    result = result && check(!isVertical(Direction::SOUTH_WEST));
    result = result && check(!isVertical(Direction::CENTRAL));
    result = result && check(!isVertical(Direction::NONE));

    result = result && check(allTraversalDirectionsSet.find(Direction::NORTH) != allTraversalDirectionsSet.end());
    result = result && check(allTraversalDirectionsSet.find(Direction::SOUTH) != allTraversalDirectionsSet.end());
    result = result && check(allTraversalDirectionsSet.find(Direction::WEST) != allTraversalDirectionsSet.end());
    result = result && check(allTraversalDirectionsSet.find(Direction::EAST) != allTraversalDirectionsSet.end());

    result = result && check(std::find(allTraversalDirections.begin(), allTraversalDirections.end(), Direction::NORTH) != allTraversalDirections.end());
    result = result && check(std::find(allTraversalDirections.begin(), allTraversalDirections.end(), Direction::SOUTH) != allTraversalDirections.end());
    result = result && check(std::find(allTraversalDirections.begin(), allTraversalDirections.end(), Direction::WEST) != allTraversalDirections.end());
    result = result && check(std::find(allTraversalDirections.begin(), allTraversalDirections.end(), Direction::EAST) != allTraversalDirections.end());

    // TODO
    //forEachTraversalDirection([](){  });
    return result;
}

bool testAreAdjacent ()
{
    bool result = true;
    // Check at 0,0 where only DOWN and RIGHT are possible
    result = result && check(areAdjacent({0,0}, {0,0}) == Direction::NONE);
    result = result && check(areAdjacent({0,0}, {1,0}) == Direction::SOUTH);
    result = result && check(areAdjacent({0,0}, {0,1}) == Direction::EAST);
    result = result && check(areAdjacent({0,0}, {2,0}) == Direction::NONE);
    result = result && check(areAdjacent({0,0}, {0,2}) == Direction::NONE);
    result = result && check(areAdjacent({0,0}, {1,1}) == Direction::NONE); // diagonal is not adjacent

    // check from 1,1
    result = result && check(areAdjacent({1,1}, {1,1}) == Direction::NONE);
    result = result && check(areAdjacent({1,1}, {2,1}) == Direction::SOUTH);
    result = result && check(areAdjacent({1,1}, {1,2}) == Direction::EAST);
    result = result && check(areAdjacent({1,1}, {1,0}) == Direction::WEST);
    result = result && check(areAdjacent({1,1}, {0,1}) == Direction::NORTH);
    result = result && check(areAdjacent({1,1}, {3,1}) == Direction::NONE);
    result = result && check(areAdjacent({1,1}, {1,3}) == Direction::NONE);
    result = result && check(areAdjacent({1,2}, {1,0}) == Direction::NONE);
    result = result && check(areAdjacent({2,1}, {0,1}) == Direction::NONE);
    result = result && check(areAdjacent({1,1}, {0,0}) == Direction::NONE); // diagonal is not adjacent
    result = result && check(areAdjacent({1,1}, {2,2}) == Direction::NONE); // diagonal is not adjacent
    return result;
}

bool testDirectionAddition ()
{
    bool result = true;

    //result = result && check(addDirections(Direction::NONE, Direction::NONE) == Direction::CENTRAL);
    result = result && check(addDirections(Direction::NONE, Direction::NONE) == Direction::NONE);

    result = result && check(addDirections(Direction::NORTH, Direction::NONE) == Direction::NORTH);
    result = result && check(addDirections(Direction::NONE, Direction::NORTH) == Direction::NORTH);
    result = result && check(addDirections(Direction::SOUTH, Direction::NONE) == Direction::SOUTH);
    result = result && check(addDirections(Direction::NONE, Direction::SOUTH) == Direction::SOUTH);
    result = result && check(addDirections(Direction::WEST, Direction::NONE) == Direction::WEST);
    result = result && check(addDirections(Direction::NONE, Direction::WEST) == Direction::WEST);
    result = result && check(addDirections(Direction::EAST, Direction::NONE) == Direction::EAST);
    result = result && check(addDirections(Direction::NONE, Direction::EAST) == Direction::EAST);

    result = result && check(addDirections(Direction::NORTH, Direction::NORTH) == Direction::NORTH);
    result = result && check(addDirections(Direction::SOUTH, Direction::SOUTH) == Direction::SOUTH);
    result = result && check(addDirections(Direction::WEST, Direction::WEST) == Direction::WEST);
    result = result && check(addDirections(Direction::EAST, Direction::EAST) == Direction::EAST);

    result = result && check(addDirections(Direction::NORTH, Direction::WEST) == Direction::NORTH_WEST);
    result = result && check(addDirections(Direction::WEST, Direction::NORTH) == Direction::NORTH_WEST);
    result = result && check(addDirections(Direction::SOUTH, Direction::WEST) == Direction::SOUTH_WEST);
    result = result && check(addDirections(Direction::WEST, Direction::SOUTH) == Direction::SOUTH_WEST);
    result = result && check(addDirections(Direction::NORTH, Direction::EAST) == Direction::NORTH_EAST);
    result = result && check(addDirections(Direction::EAST, Direction::NORTH) == Direction::NORTH_EAST);
    result = result && check(addDirections(Direction::SOUTH, Direction::EAST) == Direction::SOUTH_EAST);
    result = result && check(addDirections(Direction::EAST, Direction::SOUTH) == Direction::SOUTH_EAST);

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
    result = result && testDirectionAddition();
    return result ? 0 : 1;
}
