#include "Direction.h"

#include <cmath>

/**
 * @return string representation of direction
 */
const char * asString (Direction d) noexcept
{
    switch (d)
    {
        case UP:    return "up";
        case DOWN:  return "down";
        case LEFT:  return "left";
        case RIGHT: return "right";
        case NONE:  return "none";
    }
    return nullptr;
}

/**
 * Return coordinate resulting in moving one step in the given direction, from a start coordinate.
 * @param start         Starting coordinate
 * @param direction     Direction to move
 * @return true if coordinate changed
 * @return false if the coordinate would become invalid in that direction
 */
bool coordinateChange (Coordinate & start, Direction direction) noexcept
{
    unsigned r = start[0];
    unsigned c = start[1];
    switch (direction)
    {
        case Direction::UP:
            if (r < 1)
                return false;
            --r;
            break;
        case Direction::DOWN:
            ++r;
            break;
        case Direction::LEFT:
            if (c < 1)
                return false;
            --c;
            break;
        case Direction::RIGHT:
            ++c;
            break;
        case Direction::NONE:
            return true;
    }
    start = {r,c};
    return true;
}

/**
 * Return coordinate resulting in moving one step in the given direction, from a start coordinate.
 * @param start         Starting coordinate
 * @param direction     Direction to move
 * @return true if coordinate changed
 * @return false if the coordinate would become invalid in that direction
 */
bool coordinateChange (Coordinate & start, Adjacency direction) noexcept
{
    Coordinate c = start; // local copy until change confirmed ok
    switch (direction)
    {
        case ADJACENT_NORTH_WEST:
            if (!coordinateChange(c, Direction::UP))
                return false;
            if (!coordinateChange(c, Direction::LEFT))
                return false;
            start = c;
            return true;
        case ADJACENT_NORTH:
            return coordinateChange(start, Direction::UP);
        case ADJACENT_NORTH_EAST:
            if (!coordinateChange(c, Direction::UP))
                return false;
            if (!coordinateChange(c, Direction::RIGHT))
                return false;
            start = c;
            return true;
        case ADJACENT_WEST:
            return coordinateChange(start, Direction::LEFT);
        case ADJACENT_CENTRAL:
            return true;
        case ADJACENT_EAST:
            return coordinateChange(start, Direction::RIGHT);
        case ADJACENT_SOUTH_WEST:
            if (!coordinateChange(c, Direction::DOWN))
                return false;
            if (!coordinateChange(c, Direction::LEFT))
                return false;
            start = c;
            return true;
        case ADJACENT_SOUTH:
            return coordinateChange(start, Direction::DOWN);
        case ADJACENT_SOUTH_EAST:
            if (!coordinateChange(c, Direction::DOWN))
                return false;
            if (!coordinateChange(c, Direction::RIGHT))
                return false;
            start = c;
            return true;
    }
    [[unlikely]]
    return true;
}

/**
 * Check if coordinates are adjacent.
 * @return direction from coord1 to coord2 if they are adjacent. Direction::NONE if not adjacent
 */
Direction areAdjacent (Coordinate coord1, Coordinate coord2) noexcept
{
    if (coord1 == coord2)
        return Direction::NONE;
    unsigned r1 = coord1[0], r2 = coord2[0];
    unsigned c1 = coord1[1], c2 = coord2[1];
    Direction rowDirection = Direction::NONE;
    if (r1 != r2)
    {
        if (r1 < r2)  // row index increasing = DOWN
        {
            if (r2 - r1 > 1)
                return Direction::NONE;
            rowDirection = Direction::DOWN;
        }
        else if (r1 > r2)
        {
            if (r1 - r2 > 1)
                return Direction::NONE;
            rowDirection = Direction::UP;
        }
    }
    Direction colDirection = Direction::NONE;
    if (c1 != c2)
    {
        if (c1 < c2)  // column index increasing = RIGHT
        {
            if (c2 - c1 > 1)
                return Direction::NONE;
            colDirection = Direction::RIGHT;
        }
        else if (c1 > c2)
        {
            if (c1 - c2 > 1)
                return Direction::NONE;
            colDirection = Direction::LEFT;
        }
    }
    if (rowDirection == Direction::NONE)
        return colDirection;
    if (colDirection == Direction::NONE)
        return rowDirection;
    return Direction::NONE; // diagonal
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
