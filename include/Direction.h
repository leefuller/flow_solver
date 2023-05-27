#ifndef DIRECTION_H
#define DIRECTION_H

#include <functional>
#include <iostream>
#include <set>
#include <array>
#include <vector>

/*
 * Although traversal of cells allows 4 directions, adjacency view includes the cells diagonally adjacent.
 * The order here is important. The first 4 entries list the directions for traversal.
 */
enum Direction
{
    NORTH,
    SOUTH,
    WEST,
    EAST,

    NORTH_WEST,
    NORTH_EAST,
    CENTRAL,
    SOUTH_WEST,
    SOUTH_EAST,

    NONE // Listed after the directions that are used as array indexes
};

/** All traversal directions (excluding NONE) indexed by the Direction */
const std::array<Direction, 4> allTraversalDirections =
    { Direction::NORTH, Direction::SOUTH, Direction::WEST, Direction::EAST };
/** All traversal directions as a set, excluding NONE. */
const std::set<Direction> allTraversalDirectionsSet =
    { Direction::NORTH, Direction::SOUTH, Direction::WEST, Direction::EAST };

using Coordinate = std::array<int, 2>;
using Route = std::vector<Coordinate>;

inline Coordinate createCoordinate (int r, int c) noexcept
{ return {r, c}; }

inline std::ostream & operator<< (std::ostream & os, Coordinate c) noexcept
{
    os << '{' << c[0] << ',' << c[1] << '}';
    return os;
}

/** @return true if coordinate exists in route */
inline bool coordinateInRoute (const Coordinate & coord, const Route & route) noexcept
{ return std::find(std::begin(route), std::end(route), coord) != std::end(route); }

std::ostream & operator<< (std::ostream & os, const Route & route) noexcept;

/**
 * Execute a function for each traversal Direction, except NONE
 * @param f         function to execute for every direction
 */
inline void forEachTraversalDirection (std::function<void(Direction d)> * f)
{
    for (Direction direction : allTraversalDirections)
        (*f)(direction);
}

const char * asString (Direction d) noexcept;

inline std::ostream & operator<< (std::ostream & os, Direction d) noexcept
{
    os << asString(d);
    return os;
}

/** @return true if the direction is vertical. */
inline bool isVertical (Direction d) noexcept
{ return d == Direction::NORTH || d == Direction::SOUTH; }

inline bool isDiagonal (Direction d) noexcept
{
    return d == Direction::NORTH_EAST ||
           d == Direction::NORTH_WEST ||
           d == Direction::SOUTH_EAST ||
           d == Direction::SOUTH_WEST;
}

/** @return the opposite direction to d */
inline Direction opposite (Direction d) noexcept
{
    switch (d)
    {
        case NORTH_WEST:   return SOUTH_EAST;
        case NORTH:        return SOUTH;
        case NORTH_EAST:   return SOUTH_WEST;
        case WEST:         return EAST;
        case CENTRAL:      return CENTRAL;
        case EAST:         return WEST;
        case SOUTH_WEST:   return NORTH_EAST;
        case SOUTH:        return NORTH;
        case SOUTH_EAST:   return NORTH_WEST;
        case NONE:         return NONE;
    }
    [[unlikely]]
    return NONE;
}

Direction areAdjacent (Coordinate c1, Coordinate c2) noexcept;

Direction addDirections (Direction d1, Direction d2);

bool coordinateChange (Coordinate & start, Direction direction, unsigned distance = 1) noexcept;

#endif
