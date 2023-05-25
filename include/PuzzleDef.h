#ifndef PUZZLEDEF_H
#define PUZZLEDEF_H

#include <vector>
#include <memory>

#include "Direction.h"
#include "Cell.h"

class Puzzle;
class Cell;

using PuzzleRow = std::vector<std::shared_ptr<Cell>>;

/**
 * Definition of a puzzle.
 * The puzzle is a matrix of cells.
 * Initially, most cells are empty, and others contain fixed pipe endpoints, which are not movable.
 *
 * Able to parse and validate a puzzle definition from string.
 * Generator of a Puzzle object.
 */
class PuzzleDefinition
{
    public:

      PuzzleDefinition () = delete;

      ~PuzzleDefinition()
      {}

      PuzzleDefinition (const char * puzzleDef) noexcept(false)
      {
          parsePuzzleDef(puzzleDef);
          validatePuzzle();
      }

      PuzzleDefinition (const PuzzleDefinition&) = delete; // prevent copy construct
      PuzzleDefinition (PuzzleDefinition && p) = delete; // prevent move construct
      PuzzleDefinition & operator= (const PuzzleDefinition &) = delete; // prevent copy assignment
      PuzzleDefinition & operator= (PuzzleDefinition &&) = delete; // prevent move assignment

      /** @return identifiers for all pipes in the puzzle */
      const std::set<PipeId> getPipeIds () const noexcept
      { return m_pipeIds; }

      std::shared_ptr<Puzzle> generatePuzzle () const;

      std::vector<PuzzleRow> generateRows () const;

      /** @return number of rows in the puzzle */
      unsigned getNumRows () const noexcept
      { return m_puzzleRows.size(); }

      /** @return number of columns in the puzzle */
      unsigned getNumCols () const noexcept
      { return m_puzzleRows[0].size(); }

      bool passCoordinateRangeCheck (Coordinate coord) const noexcept;
      bool isCellReachable (Coordinate coord) const noexcept;
      bool isCoordinateChangeValid (Coordinate coord, Adjacency adj) const noexcept;

      std::set<Direction> getConnectedDirections (Coordinate coord) const;

      Coordinate findPipeEnd (PipeId id, PipeEnd end) const noexcept(false);

      bool isEndpoint (Coordinate coord) const noexcept;

    private:
      std::shared_ptr<const Cell> getConstCellAtCoordinate(Coordinate coord, bool rangeCheck = false) const noexcept(false);
      Cell * getCellAtCoordinate(Coordinate coord, bool rangeCheck = false) noexcept(false);
      std::shared_ptr<const Cell> getConstCellAdjacent (Coordinate coord, Direction direction) const noexcept;
      Cell * getCellAdjacent (Coordinate coord, Direction direction) noexcept;

      void parsePuzzleDef (const char * puzzleDef);
      std::vector<CellBorder> parseHorizontalWall (const char ** pDef, std::vector<std::unique_ptr<Cell>> * cellsAbove);
      std::vector<std::unique_ptr<Cell>> parseHorizontalCells (const char ** pDef, const std::vector<CellBorder> & borderAbove);

      void validatePuzzle ();

      std::vector<std::vector<std::unique_ptr<Cell>>> m_puzzleRows;

      std::set<PipeId> m_pipeIds;
};

#endif