#include "../include/Direction.h"
#include "../include/exceptions.h"
#include <gtest/gtest.h>

TEST(direction_test, coordinate_change)
{
    Coordinate coord = createCoordinate(2, 6);
    EXPECT_TRUE(coordinateChange(coord, Direction::NORTH));
    EXPECT_EQ(coord[0], 1);
    EXPECT_TRUE(coordinateChange(coord, Direction::SOUTH));
    EXPECT_EQ(coord[0], 2);
    EXPECT_EQ(coord[1], 6);
    EXPECT_TRUE(coordinateChange(coord, Direction::WEST));
    EXPECT_EQ(coord[0], 2);
    EXPECT_EQ(coord[1], 5);
    EXPECT_TRUE(coordinateChange(coord, Direction::EAST));
    EXPECT_EQ(coord[0], 2);
    EXPECT_EQ(coord[1], 6);

    EXPECT_TRUE(coordinateChange(coord, Direction::NORTH));
    EXPECT_EQ(coord[0], 1);
    EXPECT_EQ(coord[1], 6);
    EXPECT_TRUE(coordinateChange(coord, Direction::WEST));
    EXPECT_EQ(coord[0], 1);
    EXPECT_EQ(coord[1], 5);
    EXPECT_TRUE(coordinateChange(coord, Direction::SOUTH));
    EXPECT_EQ(coord[0], 2);
    EXPECT_EQ(coord[1], 5);
    EXPECT_TRUE(coordinateChange(coord, Direction::NORTH_EAST));
    EXPECT_EQ(coord[0], 1);
    EXPECT_EQ(coord[1], 6);
    EXPECT_TRUE(coordinateChange(coord, Direction::NORTH_WEST));
    EXPECT_EQ(coord[0], 0);
    EXPECT_EQ(coord[1], 5);
    EXPECT_TRUE(coordinateChange(coord, Direction::EAST));
    EXPECT_EQ(coord[0], 0);
    EXPECT_EQ(coord[1], 6);
    EXPECT_TRUE(coordinateChange(coord, Direction::SOUTH_EAST));
    EXPECT_EQ(coord[0], 1);
    EXPECT_EQ(coord[1], 7);
    EXPECT_TRUE(coordinateChange(coord, Direction::SOUTH_WEST));
    EXPECT_EQ(coord[0], 2);
    EXPECT_EQ(coord[1], 6);

    coord = createCoordinate(1, 1);
    EXPECT_TRUE(coordinateChange(coord, Direction::WEST));
    EXPECT_TRUE(coord[0] == 1);
    EXPECT_TRUE(coord[1] == 0);
    EXPECT_TRUE(!coordinateChange(coord, Direction::WEST));
    EXPECT_TRUE(coord[0] == 1);
    EXPECT_TRUE(coord[1] == 0);
    EXPECT_TRUE(coordinateChange(coord, Direction::NORTH));
    EXPECT_TRUE(coord[0] == 0);
    EXPECT_TRUE(coord[1] == 0);
    EXPECT_TRUE(!coordinateChange(coord, Direction::NORTH));
    EXPECT_TRUE(coord[0] == 0);
    EXPECT_TRUE(coord[1] == 0);

    coord = createCoordinate(4, 7);
    EXPECT_TRUE(coordinateChange(coord, Direction::NORTH, 2));
    EXPECT_TRUE(coord[0] == 2);
    EXPECT_TRUE(coord[1] == 7);
    EXPECT_TRUE(coordinateChange(coord, Direction::EAST, 2));
    EXPECT_TRUE(coord[0] == 2);
    EXPECT_TRUE(coord[1] == 9);
    EXPECT_TRUE(coordinateChange(coord, Direction::SOUTH, 2));
    EXPECT_TRUE(coord[0] == 4);
    EXPECT_TRUE(coord[1] == 9);
    EXPECT_TRUE(coordinateChange(coord, Direction::WEST, 3));
    EXPECT_TRUE(coord[0] == 4);
    EXPECT_TRUE(coord[1] == 6);
}

TEST(direction_test, DirectionAsString)
{
    EXPECT_STREQ(asString(Direction::NORTH), "north");
    EXPECT_STREQ(asString(Direction::SOUTH), "south");
    EXPECT_STREQ(asString(Direction::WEST), "west");
    EXPECT_STREQ(asString(Direction::EAST), "east");
    EXPECT_STREQ(asString(Direction::NONE), "none");
}

TEST(direction_test, Direction)
{
    EXPECT_TRUE(opposite(Direction::NORTH) == Direction::SOUTH);
    EXPECT_TRUE(opposite(Direction::SOUTH) == Direction::NORTH);
    EXPECT_TRUE(opposite(Direction::WEST) == Direction::EAST);
    EXPECT_TRUE(opposite(Direction::EAST) == Direction::WEST);

    EXPECT_TRUE(opposite(Direction::NORTH_WEST) == Direction::SOUTH_EAST);
    EXPECT_TRUE(opposite(Direction::NORTH) == Direction::SOUTH);
    EXPECT_TRUE(opposite(Direction::NORTH_EAST) == Direction::SOUTH_WEST);
    EXPECT_TRUE(opposite(Direction::WEST) == Direction::EAST);
    EXPECT_TRUE(opposite(Direction::CENTRAL) == Direction::CENTRAL);
    EXPECT_TRUE(opposite(Direction::EAST) == Direction::WEST);
    EXPECT_TRUE(opposite(Direction::SOUTH_WEST) == Direction::NORTH_EAST);
    EXPECT_TRUE(opposite(Direction::SOUTH) == Direction::NORTH);
    EXPECT_TRUE(opposite(Direction::SOUTH_EAST) == Direction::NORTH_WEST);
    EXPECT_TRUE(opposite(Direction::NONE) == Direction::NONE);

    EXPECT_TRUE(isVertical(Direction::NORTH));
    EXPECT_TRUE(isVertical(Direction::SOUTH));
    EXPECT_TRUE(!isVertical(Direction::WEST));
    EXPECT_TRUE(!isVertical(Direction::EAST));
    EXPECT_TRUE(!isVertical(Direction::NORTH_EAST));
    EXPECT_TRUE(!isVertical(Direction::NORTH_WEST));
    EXPECT_TRUE(!isVertical(Direction::SOUTH_EAST));
    EXPECT_TRUE(!isVertical(Direction::SOUTH_WEST));
    EXPECT_TRUE(!isVertical(Direction::CENTRAL));
    EXPECT_TRUE(!isVertical(Direction::NONE));

    EXPECT_TRUE(allTraversalDirectionsSet.find(Direction::NORTH) != allTraversalDirectionsSet.end());
    EXPECT_TRUE(allTraversalDirectionsSet.find(Direction::SOUTH) != allTraversalDirectionsSet.end());
    EXPECT_TRUE(allTraversalDirectionsSet.find(Direction::WEST) != allTraversalDirectionsSet.end());
    EXPECT_TRUE(allTraversalDirectionsSet.find(Direction::EAST) != allTraversalDirectionsSet.end());

    EXPECT_TRUE(std::find(allTraversalDirections.begin(), allTraversalDirections.end(), Direction::NORTH) != allTraversalDirections.end());
    EXPECT_TRUE(std::find(allTraversalDirections.begin(), allTraversalDirections.end(), Direction::SOUTH) != allTraversalDirections.end());
    EXPECT_TRUE(std::find(allTraversalDirections.begin(), allTraversalDirections.end(), Direction::WEST) != allTraversalDirections.end());
    EXPECT_TRUE(std::find(allTraversalDirections.begin(), allTraversalDirections.end(), Direction::EAST) != allTraversalDirections.end());
}

TEST(direction_test, rotation)
{
    EXPECT_TRUE(rotateLeft(Direction::NORTH) == Direction::NORTH_WEST);
    EXPECT_TRUE(rotateLeft(Direction::NORTH_WEST) == Direction::WEST);
    EXPECT_TRUE(rotateLeft(Direction::WEST) == Direction::SOUTH_WEST);
    EXPECT_TRUE(rotateLeft(Direction::SOUTH_WEST) == Direction::SOUTH);
    EXPECT_TRUE(rotateLeft(Direction::SOUTH) == Direction::SOUTH_EAST);
    EXPECT_TRUE(rotateLeft(Direction::SOUTH_EAST) == Direction::EAST);
    EXPECT_TRUE(rotateLeft(Direction::EAST) == Direction::NORTH_EAST);
    EXPECT_TRUE(rotateLeft(Direction::NORTH_EAST) == Direction::NORTH);
    EXPECT_TRUE(rotateLeft(Direction::NONE) == Direction::NONE);
    EXPECT_TRUE(rotateLeft(Direction::CENTRAL) == Direction::CENTRAL);

    EXPECT_TRUE(rotateRight(Direction::NORTH) == Direction::NORTH_EAST);
    EXPECT_TRUE(rotateRight(Direction::NORTH_WEST) == Direction::NORTH);
    EXPECT_TRUE(rotateRight(Direction::WEST) == Direction::NORTH_WEST);
    EXPECT_TRUE(rotateRight(Direction::SOUTH_WEST) == Direction::WEST);
    EXPECT_TRUE(rotateRight(Direction::SOUTH) == Direction::SOUTH_WEST);
    EXPECT_TRUE(rotateRight(Direction::SOUTH_EAST) == Direction::SOUTH);
    EXPECT_TRUE(rotateRight(Direction::EAST) == Direction::SOUTH_EAST);
    EXPECT_TRUE(rotateRight(Direction::NORTH_EAST) == Direction::EAST);
    EXPECT_TRUE(rotateRight(Direction::NONE) == Direction::NONE);
    EXPECT_TRUE(rotateRight(Direction::CENTRAL) == Direction::CENTRAL);
}

TEST(direction_test, adjacency)
{
    // Check at 0,0 where only DOWN and RIGHT are possible
    EXPECT_TRUE(areAdjacent({0,0}, {0,0}) == Direction::NONE);
    EXPECT_TRUE(areAdjacent({0,0}, {1,0}) == Direction::SOUTH);
    EXPECT_TRUE(areAdjacent({0,0}, {0,1}) == Direction::EAST);
    EXPECT_TRUE(areAdjacent({0,0}, {2,0}) == Direction::NONE);
    EXPECT_TRUE(areAdjacent({0,0}, {0,2}) == Direction::NONE);
    EXPECT_TRUE(areAdjacent({0,0}, {1,1}) == Direction::NONE); // diagonal is not adjacent

    // check from 1,1
    EXPECT_TRUE(areAdjacent({1,1}, {1,1}) == Direction::NONE);
    EXPECT_TRUE(areAdjacent({1,1}, {2,1}) == Direction::SOUTH);
    EXPECT_TRUE(areAdjacent({1,1}, {1,2}) == Direction::EAST);
    EXPECT_TRUE(areAdjacent({1,1}, {1,0}) == Direction::WEST);
    EXPECT_TRUE(areAdjacent({1,1}, {0,1}) == Direction::NORTH);
    EXPECT_TRUE(areAdjacent({1,1}, {3,1}) == Direction::NONE);
    EXPECT_TRUE(areAdjacent({1,1}, {1,3}) == Direction::NONE);
    EXPECT_TRUE(areAdjacent({1,2}, {1,0}) == Direction::NONE);
    EXPECT_TRUE(areAdjacent({2,1}, {0,1}) == Direction::NONE);
    EXPECT_TRUE(areAdjacent({1,1}, {0,0}) == Direction::NONE); // diagonal is not adjacent
    EXPECT_TRUE(areAdjacent({1,1}, {2,2}) == Direction::NONE); // diagonal is not adjacent
}

TEST(direction_test, addition)
{
    EXPECT_TRUE(addDirections(Direction::NONE, Direction::NONE) == Direction::NONE);

    EXPECT_TRUE(addDirections(Direction::NORTH, Direction::NONE) == Direction::NORTH);
    EXPECT_TRUE(addDirections(Direction::NONE, Direction::NORTH) == Direction::NORTH);
    EXPECT_TRUE(addDirections(Direction::SOUTH, Direction::NONE) == Direction::SOUTH);
    EXPECT_TRUE(addDirections(Direction::NONE, Direction::SOUTH) == Direction::SOUTH);
    EXPECT_TRUE(addDirections(Direction::WEST, Direction::NONE) == Direction::WEST);
    EXPECT_TRUE(addDirections(Direction::NONE, Direction::WEST) == Direction::WEST);
    EXPECT_TRUE(addDirections(Direction::EAST, Direction::NONE) == Direction::EAST);
    EXPECT_TRUE(addDirections(Direction::NONE, Direction::EAST) == Direction::EAST);

    EXPECT_TRUE(addDirections(Direction::NORTH, Direction::NORTH) == Direction::NORTH);
    EXPECT_TRUE(addDirections(Direction::SOUTH, Direction::SOUTH) == Direction::SOUTH);
    EXPECT_TRUE(addDirections(Direction::WEST, Direction::WEST) == Direction::WEST);
    EXPECT_TRUE(addDirections(Direction::EAST, Direction::EAST) == Direction::EAST);

    EXPECT_TRUE(addDirections(Direction::NORTH, Direction::WEST) == Direction::NORTH_WEST);
    EXPECT_TRUE(addDirections(Direction::WEST, Direction::NORTH) == Direction::NORTH_WEST);
    EXPECT_TRUE(addDirections(Direction::SOUTH, Direction::WEST) == Direction::SOUTH_WEST);
    EXPECT_TRUE(addDirections(Direction::WEST, Direction::SOUTH) == Direction::SOUTH_WEST);
    EXPECT_TRUE(addDirections(Direction::NORTH, Direction::EAST) == Direction::NORTH_EAST);
    EXPECT_TRUE(addDirections(Direction::EAST, Direction::NORTH) == Direction::NORTH_EAST);
    EXPECT_TRUE(addDirections(Direction::SOUTH, Direction::EAST) == Direction::SOUTH_EAST);
    EXPECT_TRUE(addDirections(Direction::EAST, Direction::SOUTH) == Direction::SOUTH_EAST);
}

TEST(direction_test, corner1)
{
    std::array<unsigned, 4> gaps = { 0, 0, 0, 0 }; // north, south, west, east
    EXPECT_TRUE(cornerDirection(gaps) == Direction::NONE);
    gaps = { 0, 0, 3, 4 };
    EXPECT_TRUE(cornerDirection(gaps) == Direction::NONE);
    gaps = { 1, 2, 0, 0 };
    EXPECT_TRUE(cornerDirection(gaps) == Direction::NONE);
    gaps = { 1, 0, 1, 0 };
    EXPECT_TRUE(cornerDirection(gaps) == Direction::SOUTH_EAST);
    gaps = { 1, 0, 0, 1 };
    EXPECT_TRUE(cornerDirection(gaps) == Direction::SOUTH_WEST);
    gaps = { 0, 1, 1, 0 };
    EXPECT_TRUE(cornerDirection(gaps) == Direction::NORTH_EAST);
    gaps = { 0, 1, 0, 1 };
    EXPECT_TRUE(cornerDirection(gaps) == Direction::NORTH_WEST);
}

TEST(direction_test, directionBetweenCoordinates)
{
     // Coordinates are row, column
    EXPECT_TRUE(getDirectionBetweenCoordinates({0,0}, {0,0}) == Direction::NONE);
    EXPECT_TRUE(getDirectionBetweenCoordinates({1,1}, {1,1}) == Direction::NONE);

    EXPECT_TRUE(getDirectionBetweenCoordinates({0,0}, {0,1}) == Direction::EAST);
    EXPECT_TRUE(getDirectionBetweenCoordinates({0,0}, {0,50}) == Direction::EAST);
    EXPECT_TRUE(getDirectionBetweenCoordinates({10,20}, {10,50}) == Direction::EAST);
    EXPECT_TRUE(getDirectionBetweenCoordinates({10,-20}, {10,-5}) == Direction::EAST);

    EXPECT_TRUE(getDirectionBetweenCoordinates({0,0}, {0,-1}) == Direction::WEST);
    EXPECT_TRUE(getDirectionBetweenCoordinates({0,0}, {0,-50}) == Direction::WEST);
    EXPECT_TRUE(getDirectionBetweenCoordinates({10,20}, {10,5}) == Direction::WEST);
    EXPECT_TRUE(getDirectionBetweenCoordinates({10,-20}, {10,-23}) == Direction::WEST);

    EXPECT_TRUE(getDirectionBetweenCoordinates({0,0}, {1,0}) == Direction::SOUTH);
    EXPECT_TRUE(getDirectionBetweenCoordinates({0,0}, {50,0}) == Direction::SOUTH);
    EXPECT_TRUE(getDirectionBetweenCoordinates({10,20}, {50,20}) == Direction::SOUTH);
    EXPECT_TRUE(getDirectionBetweenCoordinates({-10,20}, {-5,20}) == Direction::SOUTH);

    EXPECT_TRUE(getDirectionBetweenCoordinates({0,0}, {-1,0}) == Direction::NORTH);
    EXPECT_TRUE(getDirectionBetweenCoordinates({0,0}, {-50,0}) == Direction::NORTH);
    EXPECT_TRUE(getDirectionBetweenCoordinates({10,20}, {-50,20}) == Direction::NORTH);
    EXPECT_TRUE(getDirectionBetweenCoordinates({-10,20}, {-50,20}) == Direction::NORTH);

    EXPECT_TRUE(getDirectionBetweenCoordinates({0,0}, {1,1}) == Direction::SOUTH_EAST);
    EXPECT_TRUE(getDirectionBetweenCoordinates({0,0}, {-1,1}) == Direction::NORTH_EAST);
    EXPECT_TRUE(getDirectionBetweenCoordinates({0,0}, {-1,-1}) == Direction::NORTH_WEST);
    EXPECT_TRUE(getDirectionBetweenCoordinates({0,0}, {1,-1}) == Direction::SOUTH_WEST);

    EXPECT_THROW(getDirectionBetweenCoordinates({0,0}, {1,2}), InvalidOperation);
}
