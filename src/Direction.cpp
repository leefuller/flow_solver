#include "Direction.h"
//#include "PuzzleException.h"
#include "exceptions.h"

#include <cmath>

/**
 * @return string representation of direction
 */
const char * asString (Direction d) noexcept
{
    switch (d)
    {
        case NORTH:         return "north";
        case NORTH_EAST:    return "north east";
        case NORTH_WEST:    return "north west";
        case SOUTH:         return "south";
        case SOUTH_EAST:    return "south east";
        case SOUTH_WEST:    return "south west";
        case EAST:          return "east";
        case WEST:          return "west";
        case CENTRAL:       return "central";
        case NONE:          return "none";
    }
    return nullptr;
}

/**
 * Create a coordinate corresponding to a change of coordinate due to given direction.
 * ie. Adding the result to a puzzle coordinate will give the new puzzle coordinate.
 * @return result as a coordinate representing the change. (Not representing a puzzle coordinate)
 */
static Coordinate createCoordinateChange (Direction d, int distance = 1)
{
    switch (d)
    {
        case NORTH:         return {-distance, 0};
        case SOUTH:         return { distance, 0};
        case EAST:          return { 0, distance};
        case WEST:          return { 0,-distance};
        case NORTH_EAST:    return {-distance, distance};
        case NORTH_WEST:    return {-distance,-distance};
        case SOUTH_EAST:    return { distance, distance};
        case SOUTH_WEST:    return { distance,-distance};
        case CENTRAL:       return { 0, 0};
        case NONE:          return { 0, 0};
    }
    throw std::invalid_argument("invalid direction");
}

/**
 * Get coordinate resulting in moving in the given direction, from a start coordinate.
 * @param start         Starting coordinate
 * @param direction     Direction to move
 * @param distance      Number of steps
 * @return true if coordinate changed, or direction is NONE
 * @return false if the coordinate would become invalid in that direction
 */
bool coordinateChange (Coordinate & start, Direction direction, unsigned distance) noexcept
{
    if (direction == Direction::NONE)
        return true;
    Coordinate c = start; // local copy until change confirmed ok
    Coordinate change = createCoordinateChange(direction, distance);
    c[0] = c[0] + change[0];
    c[1] = c[1] + change[1];
    if (c[0] < 0 || c[1] < 0)
        return false;
    start = c;
    return true;
}

/**
 * Check if coordinates are adjacent.
 * @return direction from start to end if they are adjacent. Direction::NONE if not adjacent
 */
Direction areAdjacent (Coordinate start, Coordinate end) noexcept
{
    if (start == end)
        return Direction::NONE;
    int rStart = start[0], rEnd = end[0];
    int cStart = start[1], cEnd = end[1];
    Coordinate diff = { abs(rStart - rEnd), abs(cStart - cEnd) };
    // Adjacent means the difference between coordinates is 1
    if (diff[0] > 1 || diff[1] > 1)
        return Direction::NONE;
    if (!diff[0]) // Difference is in column (east or west)
        return cEnd < cStart ? Direction::WEST : Direction::EAST;
    if (!diff[1]) // Difference is in row (north or south)
        return rEnd < rStart ? Direction::NORTH : Direction::SOUTH;
    // Diagonal
    /*
    if (rEnd < rStart)
        return cEnd < cStart ? Direction::NORTH_WEST : Direction::NORTH_EAST;
    return cEnd < cStart ? Direction::SOUTH_WEST : Direction::SOUTH_EAST;
    */
    return Direction::NONE; // Diagonal is not adjacent
}

/**
 * Add 2 directions together.
 * If they are opposite, the result is Direction::NONE.
 * Example: addDirections(NORTH, EAST) will return NORTH_EAST
 * @throw exception if the result would not be one of the 9 valid directions.
 */
Direction addDirections (Direction d1, Direction d2)
{
    if (d1 == opposite(d2))
        return Direction::NONE;
    if (d1 == Direction::NONE || d1 == Direction::CENTRAL)
        return d2;
    if (d1 == d2 || d2 == Direction::NONE || d2 == Direction::CENTRAL)
        return d1;
    // Directions are not the same, and not opposite.

    Coordinate c1 = createCoordinateChange(d1);
    Coordinate c2 = createCoordinateChange(d2);
    // Add the relative coordinate changes.
    // A valid result will be between {-1,-1} and {1,1}, inclusive.
    // A value outside that range can only be achieved by an unsupported/invalid operation.
    // For example, adding EAST and SOUTH_EAST (which in the real world is EAST-SOUTH-EAST, but is not supported here)
    // produces a column change of 2.
    int r = c1[0] + c2[0];
    int c = c1[1] + c2[1];
    switch (r)
    {
        case -1: // NORTH
            if (c == -1)
                return NORTH_WEST;
            if (c == 1)
                return NORTH_EAST;
            if (c == 0)
                return NORTH;
            break;
        case 0:  // CENTRAL
            if (c == -1)
                return WEST;
            if (c == 1)
                return EAST;
            if (c == 0)
                return NONE;
            break;
        case 1:  // SOUTH
            if (c == -1)
                return SOUTH_WEST;
            if (c == 1)
                return SOUTH_EAST;
            if (c == 0)
                return SOUTH;
            break;
    }
    throw InvalidOperation("Cannot define direction from addition");
}

/**
 * Given gaps to obstructions in the 4 traversal directions,
 * determine which direction is the corner, if any.
 * A corner exists where gaps are zero in 2 traversal directions that are 90 degrees separated.
 * The direction of a corner is a diagonal one.
 * @return the direction to the corner, or Direction::NONE if there is not a corner
 */
Direction cornerDirection (std::array<unsigned, 4> gaps)
{
    unsigned countZero = std::count_if(std::begin(gaps), std::end(gaps), [](unsigned gap){ return gap == 0; });
    if (countZero != 2)
        return Direction::NONE;
    Direction result = Direction::NONE;
    for (Direction d : allTraversalDirections)
    {
        if (gaps[d] == 0)
            result = addDirections(result, d);
    }
    return result;
}

Direction rotateLeft (Direction start)
{
    switch (start)
    {
        case NORTH:     return NORTH_WEST;
        case NORTH_WEST:return WEST;
        case WEST:      return SOUTH_WEST;
        case SOUTH_WEST:return SOUTH;
        case SOUTH:     return SOUTH_EAST;
        case SOUTH_EAST:return EAST;
        case EAST:      return NORTH_EAST;
        case NORTH_EAST:return NORTH;
    }
    return start;
}

Direction rotateRight (Direction start)
{
    switch (start)
    {
        case NORTH:     return NORTH_EAST;
        case NORTH_EAST:return EAST;
        case EAST:      return SOUTH_EAST;
        case SOUTH_EAST:return SOUTH;
        case SOUTH:     return SOUTH_WEST;
        case SOUTH_WEST:return WEST;
        case WEST:      return NORTH_WEST;
        case NORTH_WEST:return NORTH;
    }
    return start;
}

std::ostream & operator<< (std::ostream & os, const Route & route) noexcept
{
    bool started = false;
    for (Coordinate c : route)
    {
        if (started)
            os << ',';
        os << c;
        started = true;
    }
    return os;
}
