#include "PuzzleRepr.h"
#include "Cell.h"

#include <iostream>

/**
 * Output representation of cell connection for direction
 */
std::ostream & outputConnectionRepr (std::ostream & os, Direction direction, CellConnection connection)
{
    switch (direction)
    {
        case Direction::NORTH:
            os << symbolsConnectorUp[connection];
            break;
        case Direction::SOUTH:
            os << symbolsConnectorDown[connection];
            break;
        case Direction::WEST:
            os << symbolsConnectorLeft[connection];
            break;
        case Direction::EAST:
            os << symbolsConnectorRight[connection];
            break;
        case Direction::NONE:
            break;
    }
    return os;
}

/**
 * Output representation of cell border for direction
 */
std::ostream & outputBorderRepr (std::ostream & os, Direction direction, CellBorder border)
{
    switch (direction)
    {
        case Direction::NORTH:
        case Direction::SOUTH:
            if (Cell::isOutputConnectorRep())
                os << (border == CellBorder::WALL ? CELL_BORDER_SYMBOL_HORIZONTAL_WALL : CELL_BORDER_SYMBOL_HORIZONTAL_OPEN);
            else
                os << (border == CellBorder::WALL ? HORIZONTAL_WALL_DEF_CH : ' ');
            break;
        case Direction::WEST:
        case Direction::EAST:
            os << (border == CellBorder::WALL ? VERTICAL_WALL_DEF_CH : ' ');
            break;
        case Direction::NONE:
            break;
    }
    return os;
}
