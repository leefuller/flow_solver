#ifndef DIRECTION_H
#define DIRECTION_H

#include <functional>
#include <iostream>
#include <set>
#include <array>
#include <vector>

/** Traversable directions */
enum Direction
{
    UP = 0,
    DOWN,
    LEFT,
    RIGHT,
    NONE // Listed after the 4 directions that are used as array indexes
};

/** All traversal directions (excluding NONE) indexed by the Direction */
const std::array<Direction, 4> allTraversalDirections =
    { Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT };
/** All traversal directions as a set, excluding NONE. */
const std::set<Direction> allTraversalDirectionsSet =
    { Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT };

using Coordinate = std::array<unsigned, 2>;
using Route = std::vector<Coordinate>;

inline Coordinate createCoordinate (unsigned r, unsigned c) noexcept
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

/** @return true if the direction is vertical. (UP or DOWN) */
inline bool isVertical (Direction d) noexcept
{ return d == Direction::UP || d == Direction::DOWN; }

/** @return the opposite direction to d */
inline Direction opposite (Direction d) noexcept
{
    if (d < Direction::LEFT)
        return d == Direction::UP ? Direction::DOWN : Direction::UP;
    return d == Direction::LEFT ? Direction::RIGHT : Direction::LEFT;
}

Direction areAdjacent (Coordinate c1, Coordinate c2) noexcept;

/**
 * Although traversal of cells allows 4 directions, adjacency view includes the cells diagonally adjacent.
 * ie. This can be used as directional indicators to all 8 cells surrounding an individual cell.
 * The order here is important, so that it can correspond to a 3x3 matrix in row & column order.
 * (Labels as compass directions for obvious difference to Direction)
 */
enum Adjacency
{
    ADJACENT_NORTH_WEST,
    ADJACENT_NORTH,
    ADJACENT_NORTH_EAST,
    ADJACENT_WEST,
    ADJACENT_CENTRAL,
    ADJACENT_EAST,
    ADJACENT_SOUTH_WEST,
    ADJACENT_SOUTH,
    ADJACENT_SOUTH_EAST,
};

inline Adjacency opposite (Adjacency a) noexcept(false)
{
    switch (a)
    {
        case ADJACENT_NORTH_WEST:   return ADJACENT_SOUTH_EAST;
        case ADJACENT_NORTH:        return ADJACENT_SOUTH;
        case ADJACENT_NORTH_EAST:   return ADJACENT_SOUTH_WEST;
        case ADJACENT_WEST:         return ADJACENT_EAST;
        case ADJACENT_CENTRAL:      return ADJACENT_CENTRAL;
        case ADJACENT_EAST:         return ADJACENT_WEST;
        case ADJACENT_SOUTH_WEST:   return ADJACENT_NORTH_EAST;
        case ADJACENT_SOUTH:        return ADJACENT_NORTH;
        case ADJACENT_SOUTH_EAST:   return ADJACENT_NORTH_WEST;
    }
    [[unlikely]]
     throw std::invalid_argument("invalid argument");
}

bool coordinateChange (Coordinate & start, Direction direction) noexcept;

bool coordinateChange (Coordinate & start, Adjacency direction) noexcept;

#endif
