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
        case Direction::UP:
            os << symbolsConnectorUp[connection];
            break;
        case Direction::DOWN:
            os << symbolsConnectorDown[connection];
            break;
        case Direction::LEFT:
            os << symbolsConnectorLeft[connection];
            break;
        case Direction::RIGHT:
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
        case Direction::UP:
        case Direction::DOWN:
            os << (border == CellBorder::WALL ? CELL_BORDER_SYMBOL_HORIZONTAL_WALL : CELL_BORDER_SYMBOL_HORIZONTAL_OPEN);
            break;
        case Direction::LEFT:
        case Direction::RIGHT:
            os << (border == CellBorder::WALL ? VERTICAL_WALL_DEF_CH : ' ');
            break;
        case Direction::NONE:
            break;
    }
    return os;
}
