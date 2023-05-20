#include "PuzzleDef.h"
#include "PuzzleException.h"
#include "PuzzleRepr.h"
#include "Puzzle.h"

#include <map>
#include <algorithm>
#include <iostream>

/**
 * Parse puzzle definition row expected to define horizontal wall(s), if any.
 * If a wall is detected, the corresponding cell in the given row above is modified to include the wall.
 * @param cellsAbove  points to the row above, or nullptr
 */
std::vector<CellBorder> PuzzleDefinition::parseHorizontalWall (const char ** pDef, std::vector<std::unique_ptr<Cell>> * cellsAbove)
{
    std::vector<CellBorder> row;
    bool skip = true; // to skip character that corresponds to where the cell definition may define a vertical wall, or not
    unsigned iCell = 0;
    for (unsigned i = 0; true; ++i)
    {
        if (skip)
        {
            skip = false;
            (*pDef)++;
            continue;
        }
        char c = *((*pDef)++);
        if (c == (HORIZONTAL_WALL_DEF_CH))  // wall
        {
            row.push_back(CellBorder::WALL);
            if (cellsAbove != nullptr)
            {
                (*cellsAbove)[iCell]->setBorder(Direction::DOWN, CellBorder::WALL);
                (*cellsAbove)[iCell]->setConnection(Direction::DOWN, CellConnection::NO_CONNECTOR);
            }
        }
        else if (c == ' ')
            row.push_back(CellBorder::OPEN);
        else if (c == (ROW_SEPARATOR_DEF_CH) || !c)
            break; // end of row
        else
        {
            std::cerr << "Invalid character " << (int)c << " (" << c << ") in horizontal wall definition at " << i << std::endl;
            throw PuzzleException("Invalid character in horizontal wall definition");
        }
        skip = true;
        ++iCell;
    }
    return row;
}

/**
 * Parse puzzle definition row expected to define horizontal cells
 * which can contain vertical walls between cells.
 */
std::vector<std::unique_ptr<Cell>> PuzzleDefinition::parseHorizontalCells (const char ** pDef, const std::vector<CellBorder> & borderAbove)
{
    auto parseCellBorder = [](char c) -> CellBorder { return c == VERTICAL_WALL_DEF_CH ? CellBorder::WALL : CellBorder::OPEN; };

    std::vector<std::unique_ptr<Cell>> row;
    std::unique_ptr<Cell> pCell = nullptr;
    unsigned iCell = 0;
    while (true)
    {
        char c = *((*pDef)++);
        if (c == (ROW_SEPARATOR_DEF_CH) || !c) // In a valid definition (expecting wall at end of row), this should not occur
        {
            // End of row, which follows the right side edge in a valid puzzle definition.
            // Parsing of wall should have added right side cell already.
            break;
        }
        if (c == (HORIZONTAL_WALL_DEF_CH))  // wall
        {
            std::cerr << "Unexpected horizontal wall parsing cell" << std::endl;
        }

        CellBorder vborder = parseCellBorder(c);
        if (pCell != nullptr) // The cell pending result of parsing border
        {
            pCell->setBorder(Direction::RIGHT, vborder);
            if (vborder == CellBorder::WALL)
                pCell->setConnection(Direction::RIGHT, CellConnection::NO_CONNECTOR);
            row.push_back(std::move(pCell));
        }
        c = *((*pDef)++);
        if (c == (ROW_SEPARATOR_DEF_CH) || !c)
            break; // end of row

        pCell = std::unique_ptr<Cell>(new Cell);  // new cell being parsed. Above has just parsed the left border.
        if (c == (UNREACHABLE_CELL_DEF_CH))
        {
            pCell->setPipeId(c);
            pCell->setConnections(noConnections);
            for (Direction d : allTraversalDirections)
                pCell->setBorder(d, CellBorder::WALL);
        }
        else
        {
            pCell->setBorder(Direction::LEFT, vborder); // cell has same left border as previous right border
            pCell->setBorder(Direction::UP, borderAbove[iCell]); // set upper border from lower border of row above
            pCell->setConnection(Direction::UP, borderAbove[iCell] == CellBorder::WALL ? CellConnection::NO_CONNECTOR : CellConnection::OPEN_CONNECTOR);

            pCell->setPipeId(c == (EMPTY_CELL_DEF_CH) ? NO_PIPE_ID : c);
            if (vborder == CellBorder::WALL)
                pCell->setConnection(Direction::LEFT, CellConnection::NO_CONNECTOR);
        }
        // cell RIGHT border is determined on parsing next char
        // cell DOWN border is determined on parsing next row definiton
        ++iCell;
    }
    return row;
}

/**
 * Parse puzzle definition to generate the puzzle cells.
 * @param puzzleDef contains rows separated by comma characters
 */
void PuzzleDefinition::parsePuzzleDef (const char * puzzleDef)
{
    std::vector<std::unique_ptr<Cell>> rowAbove;
    std::vector<CellBorder> borderAbove;
    unsigned parserState = 0; // 0 to parse horizontal wall definition; 1 for row of cells
    unsigned puzzleRowNum = 0; // Number of rows of cells parsed
    const char * pDef = puzzleDef;
    for (unsigned i = 0; true; ++i)
    {
        char c = puzzleDef[i];
        if (!c || c == ROW_SEPARATOR_DEF_CH) // end of row
        {
            switch (parserState)
            {
                case 0: // parse horizontal wall
                    borderAbove = parseHorizontalWall(&pDef, puzzleRowNum == 0 ? nullptr : &rowAbove);
                    if (puzzleRowNum)
                        m_puzzleRows.push_back(std::move(rowAbove));
                    ++parserState;
                    break;
                case 1: // parse horizontal cells
                    rowAbove = parseHorizontalCells(&pDef, borderAbove);
                    ++puzzleRowNum;
                    parserState = 0;
                    break;
            }
            if (!c)
                break;
        }
    }
}

/**
 * Get the cell at the given coordinate.
 * If the coordinate is outside the puzzle, an exception is thrown if rangeCheck is true, otherwise result is undefined.
 * @param coord         Coordinate
 * @param rangeCheck    true to execute a range check for coord.
 * @return const pointer to cell
 */
std::shared_ptr<const Cell> PuzzleDefinition::getConstCellAtCoordinate (Coordinate coord, bool rangeCheck) const noexcept(false)
{
    unsigned row = coord[0];
    unsigned col = coord[1];
    if (rangeCheck)
    {
        if (row >= m_puzzleRows.size())
            throw std::invalid_argument("row out of range");
        if (col >= m_puzzleRows[row].size())
            throw std::invalid_argument("column out of range");
    }
    return std::shared_ptr<const Cell>(new Cell(m_puzzleRows[row][col]));
}

/**
 * Get the cell at the given coordinate.
 * @return pointer to cell
 */
Cell * PuzzleDefinition::getCellAtCoordinate (Coordinate coord, bool rangeCheck) noexcept(false)
{
    unsigned row = coord[0];
    unsigned col = coord[1];
    if (rangeCheck)
    {
        if (row >= m_puzzleRows.size())
            throw std::invalid_argument("row out of range");
        if (col >= m_puzzleRows[row].size())
            throw std::invalid_argument("column out of range");
    }
    return (m_puzzleRows[row][col]).get();
}

/**
 * Return the cell adjacent to the given coordinate in the given direction,
 * only if it is immediately reachable in that direction. Disregards pipes.
 * ie. if not blocked by a wall
 * @param coord     Current coordinate
 * @param d         Direction to adjacent cell
 * @return adjacent Cell if direction is open, or nullptr if the direction is blocked by a wall.
 */
Cell * PuzzleDefinition::getCellAdjacent (Coordinate coord, Direction d) noexcept
{
    Cell * pCell = getCellAtCoordinate(coord);
    if (d == NONE)
        return pCell;
    if (pCell->isBorderOpen(d) && coordinateChange(coord, d))
        return getCellAtCoordinate(coord);
    return nullptr;
}

/**
 * Return the cell adjacent to the given coordinate in the given direction,
 * only if it is immediately reachable in that direction. Disregards pipes.
 * ie. if not blocked by a wall
 * @param coord         Current coordinate
 * @param direction     Direction to adjacent cell
 * @return adjacent Cell if direction is open, or nullptr if the direction is blocked by a wall.
 */
std::shared_ptr<const Cell> PuzzleDefinition::getConstCellAdjacent (Coordinate coord, Direction d) const noexcept
{
    const std::shared_ptr<const Cell> & pCell = getConstCellAtCoordinate(coord);
    if (d == NONE)
        return pCell;
    if (pCell->isBorderOpen(d) && coordinateChange(coord, d))
        return getConstCellAtCoordinate(coord);
    return nullptr;
}

/**
 * Determine if a coordinate can ever be reached, regardless of current puzzle state.
 * @param coord     Coordinate to assess.
 * @return true if coordinate could be reached at some time
 */
bool PuzzleDefinition::isCellReachable (Coordinate coord) const noexcept
{
    if (coord[0] >= getNumRows() || coord[1] >= getNumCols())
        return false;
    return getConstCellAtCoordinate(coord)->getPipeId() != UNREACHABLE_CELL_DEF_CH;
}

bool PuzzleDefinition::passCoordinateRangeCheck (Coordinate coord) const noexcept
{
    unsigned r = coord[0];
    unsigned c = coord[1];
    return r >= 0 && r < getNumRows() && c >= 0 && c < getNumCols();
}

bool PuzzleDefinition::isCoordinateChangeValid (Coordinate coord, Adjacency adj) const noexcept
{
    if (!passCoordinateRangeCheck(coord))
        return false;
    switch (adj)
    {
        case ADJACENT_NORTH_WEST:   return coord[0] > 0 && coord[1] > 0;
        case ADJACENT_NORTH:        return coord[0] > 0;
        case ADJACENT_NORTH_EAST:   return coord[0] > 0 && coord[1] < getNumCols() - 1;
        case ADJACENT_WEST:         return coord[1] > 0;
        case ADJACENT_CENTRAL:      return true;
        case ADJACENT_EAST:         return coord[1] < getNumCols() - 1;
        case ADJACENT_SOUTH_WEST:   return coord[0] < getNumRows() - 1 && coord[1] > 0;
        case ADJACENT_SOUTH:        return coord[0] < getNumRows() - 1;
        case ADJACENT_SOUTH_EAST:   return coord[0] < getNumRows() - 1 && coord[1] < getNumCols() - 1;
    }
    [[unlikely]]
    return false;
}

/**
 * Find a particular endpoint
 * @param id    Pipe identifier
 * @param end   End identifier. (ie. find which end)
 * @return coordinate where end of pipe is found
 * @throw exception if not found
 */
Coordinate PuzzleDefinition::findPipeEnd (PipeId id, PipeEnd end) const noexcept(false)
{
    unsigned r = 0;
    for (const std::vector<std::unique_ptr<Cell>> & row : m_puzzleRows)
    {
        unsigned c = 0;
        for (const std::unique_ptr<Cell> & cell : row)
        {
            if (id == cell->getPipeId() && cell->getEndpoint() == end)
                return {r, c};
            ++c;
        }
        ++r;
    }
    throw PuzzleException("pipe end not found");
}

bool PuzzleDefinition::isEndpoint (Coordinate coord) const noexcept
{ return getConstCellAtCoordinate(coord)->isEndpoint(); }

/**
 * Get directions which can be reached directly from the cell at the given coordinate.
 * ie. Directions not blocked by a wall.
 * @return set of directions that are not blocked by a wall.
 */
std::set<Direction> PuzzleDefinition::getConnectedDirections (Coordinate coord) const
{
    //if (!passCoordinateRangeCheck(coord))
        //return ;
    std::shared_ptr<const Cell> cell = getConstCellAtCoordinate(coord);
    std::set<Direction> result;
    for (Direction direction : allTraversalDirections)
    {
        if (cell->isBorderOpen(direction))
            result.insert(direction);
    }
    return result;
}

/**
 * Create a Puzzle from the definition
 */
std::shared_ptr<Puzzle> PuzzleDefinition::generatePuzzle () const
{
    return std::shared_ptr<Puzzle>(new Puzzle(*this));
}

std::vector<PuzzleRow> PuzzleDefinition::generateRows () const
{
    std::vector<PuzzleRow> puzzleRows;
    for (const std::vector<std::unique_ptr<Cell>> & row : m_puzzleRows)
    {
        PuzzleRow destRow;
        for (const std::unique_ptr<Cell> & cell : row)
        {
            destRow.push_back(std::make_shared<Cell>(cell));
        }
        puzzleRows.push_back(destRow);
    }
    return puzzleRows;
}

/**
 Validate puzzle definition, and set endpoints.
 There must be 2 endpoints for each pipe;
 The puzzle must have a complete border.
*/
void PuzzleDefinition::validatePuzzle ()
{
    if (m_puzzleRows.size() < 1)
        throw PuzzleException("A valid puzzle definition requires at least 1 row");
    std::cout << "Validate puzzle with " << m_puzzleRows.size() << " rows." << std::endl;

    std::map<PipeId, unsigned> endpoints; // count endpoints per pipe id
    unsigned r = 0;
    for (std::vector<std::unique_ptr<Cell>> & row : m_puzzleRows)
    {
        for (unsigned c = 0; c < row.size(); ++c)
        {
            std::unique_ptr<Cell> & cell = row[c];
            cell->setCoordinate({r, c});
            if (!cell->isEmpty() && (cell->getPipeId() != UNREACHABLE_CELL_DEF_CH)) // cell is a pipe endpoint
            {
                cell->changeConnections(CellConnection::OPEN_CONNECTOR, CellConnection::OPEN_FIXTURE);
                cell->setConnection(Direction::UP, cell->getBorder(Direction::UP) == CellBorder::OPEN ? CellConnection::OPEN_FIXTURE : CellConnection::NO_CONNECTOR);
                cell->setConnection(Direction::DOWN, cell->getBorder(Direction::DOWN) == CellBorder::OPEN ? CellConnection::OPEN_FIXTURE : CellConnection::NO_CONNECTOR);
                cell->setConnection(Direction::LEFT, cell->getBorder(Direction::LEFT) == CellBorder::OPEN ? CellConnection::OPEN_FIXTURE : CellConnection::NO_CONNECTOR);
                cell->setConnection(Direction::RIGHT, cell->getBorder(Direction::RIGHT) == CellBorder::OPEN ? CellConnection::OPEN_FIXTURE : CellConnection::NO_CONNECTOR);
                unsigned count = endpoints[cell->getPipeId()];
                if (count > 1)
                    throw PuzzleException("There are more than 2 endpoints for a pipe");
                endpoints[cell->getPipeId()] = count + 1;
                cell->setEndpoint(count == 0 ? PIPE_END_1 : PIPE_END_2);
                if (cell->getEndpoint() == PIPE_END_2)
                    m_pipeIds.insert(cell->getPipeId());

                for (Direction d : allTraversalDirections)
                {
                    Cell * pCellAdjacent = getCellAdjacent({r,c}, d);
                    if (pCellAdjacent == nullptr)
                        continue;
                    if (pCellAdjacent->getPipeId() == NO_PIPE_ID)
                        continue;
                    // If adjacent cell is an endpoint for a different pipe, remove connection on the side of cell,
                    // and for this cell, remove the connection on the side of the adjacent cell.
                    if (pCellAdjacent->getPipeId() != cell->getPipeId())
                    {
                        //std::cout << "Remove connector to adjacent endpoint " << pCellAdjacent->getPipeId() << std::endl;
                        pCellAdjacent->setConnection(opposite(d), CellConnection::NO_CONNECTOR);
                        cell->setConnection(d, CellConnection::NO_CONNECTOR);
                    }
                }
            }
        }
        if (row[0]->getBorder(Direction::LEFT) != CellBorder::WALL)
            throw PuzzleException("Left border not complete");
        if (row[row.size() - 1]->getBorder(Direction::RIGHT) != CellBorder::WALL)
            throw PuzzleException("Right border not complete");
        ++r;
    }
    for (std::pair<PipeId, unsigned> endpoint : endpoints)
    {
        if (endpoint.second != 2)
            throw PuzzleException("Pipe does not have 2 endpoints");
    }

    // Check first row has top border, and last row has bottom border
    unsigned rLast = m_puzzleRows.size() - 1;
    for (unsigned c = 0; c < m_puzzleRows[0].size(); ++c)
    {
        if (isCellReachable({0, c}))
        {
            if (m_puzzleRows[0][c]->getBorder(Direction::UP) != CellBorder::WALL)
                throw PuzzleException("Top border not complete");
        }
        if (isCellReachable({rLast, c}))
        {
            if (m_puzzleRows[rLast][c]->getBorder(Direction::DOWN) != CellBorder::WALL)
                throw PuzzleException("Bottom border not complete");
        }
    }
    std::cout << "Validated puzzle with " << m_puzzleRows.size() << " rows." << std::endl;
}
