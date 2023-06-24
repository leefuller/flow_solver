#include "PuzzleDef.h"
#include "PuzzleException.h"
#include "PuzzleRepr.h"
#include "Puzzle.h"
#include "Logger.h"

#include <map>
#include <algorithm>
#include <iostream>

static Logger & logger = Logger::getDefaultLogger();

/**
 * Parse puzzle definition row expected to define horizontal wall(s), if any.
 * If a wall is detected, the corresponding cell in the given row above is modified to include the wall.
 * @param cellsAbove  points to the row above, or nullptr
 */
std::vector<CellBorder> PuzzleDefinition::parseHorizontalWall (const char ** pDef, std::vector<std::unique_ptr<Cell>> * cellsAbove)
{
    //logger << "Parse horizontal wall" << std::endl;
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
                (*cellsAbove)[iCell]->setBorder(Direction::SOUTH, CellBorder::WALL);
                (*cellsAbove)[iCell]->setConnection(Direction::SOUTH, CellConnection::NO_CONNECTOR);
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
    //logger << "Parsed horizontal wall" << std::endl;
    return row;
}

/**
 * Parse puzzle definition row expected to define horizontal cells
 * which can contain vertical walls between cells.
 */
std::vector<std::unique_ptr<Cell>> PuzzleDefinition::parseHorizontalCells (const char ** pDef, const std::vector<CellBorder> & borderAbove)
{
    //logger << "Parse horizontal cells" << std::endl;
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
            pCell->setBorder(Direction::EAST, vborder);
            if (vborder == CellBorder::WALL)
                pCell->setConnection(Direction::EAST, CellConnection::NO_CONNECTOR);
            row.push_back(std::move(pCell));
            // pCell is nullptr after move
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
            pCell->setBorder(Direction::WEST, vborder); // cell has same left border as previous right border
            pCell->setBorder(Direction::NORTH, borderAbove[iCell]); // set upper border from lower border of row above
            pCell->setConnection(Direction::NORTH, borderAbove[iCell] == CellBorder::WALL ? CellConnection::NO_CONNECTOR : CellConnection::OPEN_CONNECTOR);

            pCell->setPipeId(c == (EMPTY_CELL_DEF_CH) ? NO_PIPE_ID : c);
            if (vborder == CellBorder::WALL)
                pCell->setConnection(Direction::WEST, CellConnection::NO_CONNECTOR);
        }
        // cell EAST border is determined on parsing next char
        // cell SOUTH border is determined on parsing next row definiton
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
#if ANNOUNCE_SOLVER_DETAIL
    logger << "Parse puzzle definition" << std::endl;
#endif
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
#if ANNOUNCE_SOLVER_DETAIL
    logger << "Parsed puzzle definition" << std::endl;
#endif
}

/**
 * Get the cell at the given coordinate.
 * If the coordinate is outside the puzzle, an exception is thrown if rangeCheck is true, otherwise result is undefined.
 * @param coord         Coordinate
 * @param rangeCheck    true to execute a range check for coord.
 * @return const pointer to cell
 */
ConstCellPtr PuzzleDefinition::getConstCellAtCoordinate (Coordinate coord, bool rangeCheck) const noexcept(false)
{
    if (rangeCheck)
    {
        if (!passCoordinateRangeCheck(coord))
            throw std::invalid_argument("coordinate out of range");
    }
    return ConstCellPtr(new Cell(m_puzzleRows[coord[0]][coord[1]]));
}

/**
 * Get the cell at the given coordinate.
 * @param coord         Coordinate
 * @param rangeCheck    true to execute a range check for coord.
 * @return pointer to cell
 */
Cell * PuzzleDefinition::getCellAtCoordinate (Coordinate coord, bool rangeCheck) noexcept(false)
{
    if (rangeCheck)
    {
        if (!passCoordinateRangeCheck(coord))
            throw std::invalid_argument("coordinate out of range");
    }
    return (m_puzzleRows[coord[0]][coord[1]]).get();
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
ConstCellPtr PuzzleDefinition::getConstCellAdjacent (Coordinate coord, Direction d) const noexcept
{
    const ConstCellPtr & pCell = getConstCellAtCoordinate(coord);
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
    if (!passCoordinateRangeCheck(coord))
        return false;
    return getConstCellAtCoordinate(coord)->getPipeId() != UNREACHABLE_CELL_DEF_CH;
}

/**
 * Determine whether coordinate is inside the puzzle dimensions,
 * disregarding whether there may be a cell there, or not.
 * @param coord     Coordinate to check
 * @return true if coordinate is in the range within the puzzle
 */
bool PuzzleDefinition::passCoordinateRangeCheck (Coordinate coord) const noexcept
{
    int r = coord[0];
    int c = coord[1];
    return r >= 0 && r < getNumRows() && c >= 0 && c < getNumCols();
}

/**
 * Determine whether a coordinate change is valid, given starting coordinate and direction.
 * Disregards walls.
 * @param coord     Starting coordinate
 * @param adj       Direction
 * @return true if coordinate change would be valid
 */
bool PuzzleDefinition::isCoordinateChangeValid (Coordinate coord, Direction adj) const noexcept
{
    if (!passCoordinateRangeCheck(coord))
        return false;
    switch (adj)
    {
        case NORTH_WEST:   return coord[0] > 0 && coord[1] > 0;
        case NORTH:        return coord[0] > 0;
        case NORTH_EAST:   return coord[0] > 0 && coord[1] < getNumCols() - 1;
        case WEST:         return coord[1] > 0;
        case CENTRAL:      return true;
        case EAST:         return coord[1] < getNumCols() - 1;
        case SOUTH_WEST:   return coord[0] < getNumRows() - 1 && coord[1] > 0;
        case SOUTH:        return coord[0] < getNumRows() - 1;
        case SOUTH_EAST:   return coord[0] < getNumRows() - 1 && coord[1] < getNumCols() - 1;
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
    int r = 0;
    for (const std::vector<std::unique_ptr<Cell>> & row : m_puzzleRows)
    {
        int c = 0;
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

/** @return true if cell at coordinate is any endpoint */
bool PuzzleDefinition::isEndpoint (Coordinate coord) const noexcept
{ return getConstCellAtCoordinate(coord)->isEndpoint(); }

/**
 * From the given coordinate determine how many cells can be traversed before reaching a wall.
 * @param c     Starting coordinate
 * @param d     Traversal direction
 * @return number of empty cells between c and wall
 */
unsigned PuzzleDefinition::gapToWall (Coordinate c, Direction d) const noexcept
{
    unsigned count = 0;
    ConstCellPtr pCell = getConstCellAtCoordinate(c);
    while (1)
    {
        // check if cell has a border in that direction.
        if (pCell->getBorder(d) == CellBorder::WALL)
            break;
        if (!coordinateChange(c, d))
            break;
        pCell = getConstCellAtCoordinate(c);
        ++count;
    }
    return count;
}

/**
 * Convenience function return the gapToWall function result for each each traversal direction from a coordinate.
 * @return array indexed by Direction
 */
std::array<unsigned, 4> PuzzleDefinition::getGapsToWalls (Coordinate c) const noexcept
{
    std::array<unsigned, 4> result;
    for (Direction d : allTraversalDirections)
        result[d] = gapToWall(c, d);
    return result;
}

/**
 * Get directions which can be reached directly from the cell at the given coordinate.
 * ie. Directions not blocked by a wall.
 * @return set of directions that are not blocked by a wall.
 */
std::set<Direction> PuzzleDefinition::getConnectedDirections (Coordinate coord) const
{
    //if (!passCoordinateRangeCheck(coord))
        //return ;
    ConstCellPtr cell = getConstCellAtCoordinate(coord);
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
PuzzlePtr PuzzleDefinition::generatePuzzle () const
{
    logger << "Generate puzzle" << std::endl;
    return PuzzlePtr(new Puzzle(*this));
}

std::vector<PuzzleRow> PuzzleDefinition::generateRows () const
{
    std::vector<PuzzleRow> puzzleRows;
    for (const std::vector<std::unique_ptr<Cell>> & row : m_puzzleRows)
    {
        PuzzleRow destRow;
        for (const std::unique_ptr<Cell> & cell : row)
        {
            CellPtr p = std::make_shared<Cell>(cell);
            if (cell->isEndpoint())
                cell->setPossiblePipes(cell->getPipeId());
            destRow.push_back(p);
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
#if ANNOUNCE_SOLVER_DETAIL
    logger << "Validate puzzle with " << m_puzzleRows.size() << " rows" << std::endl;
#endif

    std::map<PipeId, unsigned> endpoints; // count endpoints per pipe id
    int r = 0;
    for (std::vector<std::unique_ptr<Cell>> & row : m_puzzleRows)
    {
        for (int c = 0; c < row.size(); ++c)
        {
            std::unique_ptr<Cell> & cell = row[c];
            cell->setCoordinate({r, c});
            if (!cell->isEmpty() && (cell->getPipeId() != UNREACHABLE_CELL_DEF_CH)) // cell is a pipe endpoint
            {
                cell->changeConnections(CellConnection::OPEN_CONNECTOR, CellConnection::OPEN_FIXTURE);
                cell->setConnection(Direction::NORTH, cell->getBorder(Direction::NORTH) == CellBorder::OPEN ? CellConnection::OPEN_FIXTURE : CellConnection::NO_CONNECTOR);
                cell->setConnection(Direction::SOUTH, cell->getBorder(Direction::SOUTH) == CellBorder::OPEN ? CellConnection::OPEN_FIXTURE : CellConnection::NO_CONNECTOR);
                cell->setConnection(Direction::WEST, cell->getBorder(Direction::WEST) == CellBorder::OPEN ? CellConnection::OPEN_FIXTURE : CellConnection::NO_CONNECTOR);
                cell->setConnection(Direction::EAST, cell->getBorder(Direction::EAST) == CellBorder::OPEN ? CellConnection::OPEN_FIXTURE : CellConnection::NO_CONNECTOR);
                unsigned count = endpoints[cell->getPipeId()];
                if (count > 1)
                    throw PuzzleException("There are more than 2 endpoints for a pipe");
                endpoints[cell->getPipeId()] = count + 1;
                cell->setEndpoint(count == 0 ? PIPE_START : PIPE_END);
                if (cell->getEndpoint() == PIPE_END)
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
                        pCellAdjacent->setConnection(opposite(d), CellConnection::NO_CONNECTOR);
                        cell->setConnection(d, CellConnection::NO_CONNECTOR);
                    }
                }
            }
        }
        if (row[0]->getBorder(Direction::WEST) != CellBorder::WALL)
            throw PuzzleException("Left border not complete");
        if (row[row.size() - 1]->getBorder(Direction::EAST) != CellBorder::WALL)
            throw PuzzleException("Right border not complete");
        ++r;
    }
    for (std::pair<PipeId, unsigned> endpoint : endpoints)
    {
        if (endpoint.second != 2)
            throw PuzzleException("Pipe does not have 2 endpoints");
    }

    // Check first row has top border, and last row has bottom border
    int rLast = m_puzzleRows.size() - 1;
    for (int c = 0; c < m_puzzleRows[0].size(); ++c)
    {
        if (isCellReachable({0, c}))
        {
            if (m_puzzleRows[0][c]->getBorder(Direction::NORTH) != CellBorder::WALL)
                throw PuzzleException("Top border not complete");
        }
        if (isCellReachable({rLast, c}))
        {
            if (m_puzzleRows[rLast][c]->getBorder(Direction::SOUTH) != CellBorder::WALL)
                throw PuzzleException("Bottom border not complete");
        }
    }
#if ANNOUNCE_SOLVER_DETAIL
    logger << "Validated puzzle with " << m_puzzleRows.size() << " rows" << std::endl;
#endif
}
