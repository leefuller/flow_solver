#ifndef PUZZLE_REPR_H
#define PUZZLE_REPR_H
/*
 * This includes representation for connectors and other symbols
 * used in outputting a full puzzle representation,
 * in addition to the few symbols needed to define a puzzle.
 */

// Symbols used in puzzle definition, and representation-------------------
#define VERTICAL_WALL_DEF_CH     '|'
#define HORIZONTAL_WALL_DEF_CH   '='
#define ROW_SEPARATOR_DEF_CH     ','
#define EMPTY_CELL_DEF_CH        '.'
#define UNREACHABLE_CELL_DEF_CH  ' '
//#define ENDPOINT_SYMBOL_LHS      '['
//#define ENDPOINT_SYMBOL_RHS      ']'

// Connection representations for horizontal ------------------------------
#define NO_CONNECTOR_SYMBOL_LHS         ' '
#define NO_CONNECTOR_SYMBOL_RHS         ' '
#define OPEN_CONNECTION_SYMBOL_LHS      '.'
#define OPEN_CONNECTION_SYMBOL_RHS      '.'
#define TEMPORARY_CONNECTION_SYMBOL_LHS '$'
#define TEMPORARY_CONNECTION_SYMBOL_RHS '$'
#define FIXTURE_CONNECTION_SYMBOL_LHS   '#'
#define FIXTURE_CONNECTION_SYMBOL_RHS   '#'
#define FIXTURE_OPEN_SYMBOL_LHS         '>'
#define FIXTURE_OPEN_SYMBOL_RHS         '<'

// Connection representations for vertical ------------------------------
// 3 chars wide because a cell uses 3 chars in the horizontal, to include horizontal connectors.
#define NO_CONNECTOR_SYMBOL_UP              "   "
#define NO_CONNECTOR_SYMBOL_DOWN            "   "
#define OPEN_CONNECTION_SYMBOL_UP           " . "
#define OPEN_CONNECTION_SYMBOL_DOWN         " . "
#define TEMPORARY_CONNECTION_SYMBOL_UP      " $ "
#define TEMPORARY_CONNECTION_SYMBOL_DOWN    " $ "
#define FIXTURE_CONNECTION_SYMBOL_UP        " # "
#define FIXTURE_CONNECTION_SYMBOL_DOWN      " # "
#define FIXTURE_OPEN_SYMBOL_UP              " v "
#define FIXTURE_OPEN_SYMBOL_DOWN            " ^ "

#define CELL_BORDER_SYMBOL_HORIZONTAL_WALL   "==="
#define CELL_BORDER_SYMBOL_HORIZONTAL_OPEN   "   "

#include <array>

// For index: NO_CONNECTOR, OPEN_CONNECTOR, TEMPORARY_CONNECTION, FIXTURE_CONNECTION, OPEN_FIXTURE

const std::array<const char*, 5>  symbolsConnectorUp =
        { NO_CONNECTOR_SYMBOL_UP, OPEN_CONNECTION_SYMBOL_UP, TEMPORARY_CONNECTION_SYMBOL_UP, FIXTURE_CONNECTION_SYMBOL_UP, FIXTURE_OPEN_SYMBOL_UP };
const std::array<const char*, 5>  symbolsConnectorDown =
        { NO_CONNECTOR_SYMBOL_DOWN, OPEN_CONNECTION_SYMBOL_DOWN, TEMPORARY_CONNECTION_SYMBOL_DOWN, FIXTURE_CONNECTION_SYMBOL_DOWN, FIXTURE_OPEN_SYMBOL_DOWN };
const std::array<const char, 5>  symbolsConnectorLeft =
        { NO_CONNECTOR_SYMBOL_LHS, OPEN_CONNECTION_SYMBOL_LHS, TEMPORARY_CONNECTION_SYMBOL_LHS, FIXTURE_CONNECTION_SYMBOL_LHS, FIXTURE_OPEN_SYMBOL_LHS };
const std::array<const char, 5>  symbolsConnectorRight =
        { NO_CONNECTOR_SYMBOL_RHS, OPEN_CONNECTION_SYMBOL_RHS, TEMPORARY_CONNECTION_SYMBOL_RHS, FIXTURE_CONNECTION_SYMBOL_RHS, FIXTURE_OPEN_SYMBOL_RHS };

#endif
