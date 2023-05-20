#ifndef PUZZLE_H
#define PUZZLE_H

#include <vector>
#include <set>
#include <map>
#include <functional>
#include <memory>
#include <ostream>

#include "PuzzleRepr.h"
#include "Direction.h"
#include "Pipe.h"
#include "PuzzleDef.h"
#include "Plumber.h"

class Cell;

using PuzzleRow = std::vector<std::shared_ptr<Cell>>;

/**
 * This class is a data class, with helper functions for getting aspects of puzzle state.
 *
 * The puzzle is a matrix of cells.
 * Initially, most cells are empty, and others contain fixed pipe endpoints, which are not movable.
 *
 * A puzzle is solved when all endpoints are connected without any pipe intersections,
 * and all cells are used, if the puzzle was properly defined.
 *
 */
class Puzzle
{
  public:

    Puzzle (const Puzzle & p);
    Puzzle (const Puzzle && p) = delete; // prevent move construct

    ~Puzzle()
    {}

    Puzzle & operator= (const Puzzle &) = delete; // prevent copy assignment
    Puzzle & operator= (const Puzzle &&) = delete; // prevent move assignment

	std::ostream & streamPuzzleMatrix (std::ostream & os) const noexcept;

	bool isCoordinateChangeValid (Coordinate from, Adjacency a) const noexcept
	{ return m_def.isCoordinateChangeValid(from, a); }

    static bool checkIfSolution (std::shared_ptr<const Puzzle> puzzle, const std::map<PipeId, Route> & s);

    /** @return a Plumber able to work on this puzzle */
    std::shared_ptr<Plumber> getPlumber () noexcept
    { return std::make_shared<Plumber>(this); }

    /**
     * Determine if a coordinate can ever be reached, regardless of current puzzle state.
     * @param c     Coordinate to assess.
     * @return true if coordinate could be reached at some time
     */
	bool isCellReachable (Coordinate c) const noexcept
	{ return m_def.isCellReachable(c); }

	// Cell lookup -----------------------

    std::shared_ptr<Cell> getCellAtCoordinate(Coordinate coord, bool rangeCheck = false) noexcept(false);
    std::shared_ptr<const Cell> getConstCellAtCoordinate(Coordinate c, bool rangeCheck = false) const noexcept(false);

    std::shared_ptr<Cell> getCellAdjacent (std::shared_ptr<const Cell> cell, Direction d) ;//noexcept;
    std::shared_ptr<const Cell> getConstCellAdjacent (std::shared_ptr<const Cell> cell, Direction d) const ;//noexcept;

	std::shared_ptr<Cell> getCellAdjacent (Coordinate c, Direction d) ;//noexcept;
	std::shared_ptr<const Cell> getConstCellAdjacent (Coordinate c, Direction d) const ;//noexcept;

	std::map<Direction, std::shared_ptr<const Cell>> getAdjacentCells (std::shared_ptr<const Cell> cell) const noexcept;

	std::array<std::shared_ptr<const Cell>, 9> getAdjacentCells (Coordinate c) const ;//noexcept;
	std::shared_ptr<Cell> getCellAdjacent (Coordinate c, Adjacency d) ;//noexcept;
    std::shared_ptr<const Cell> getConstCellAdjacent (Coordinate c, Adjacency d) const ;//noexcept;

    // Query directions ------------------------

    /**
     * Get directions which can be reached directly from the cell at the given coordinate,
     * regardless of pipes. ie. Directions not blocked by a wall.
     * @regard coord    Coordinate to assess
     * @return set of directions that are not blocked by a wall.
     */
    std::set<Direction> getConnectedDirections (Coordinate coord) const
    { return m_def.getConnectedDirections(coord); }

    // ------------------------------

	void insertRoute (PipeId id, const Route & route);
    void removeRoute ();

	bool traceRoute (PipeId idPipe, Route & route) const;
	void traceRoutes (std::map<PipeId, Route> &) const;

    // ------------------------------
	// Helpers

    void forEveryCell (std::function<void(std::shared_ptr<Cell>)> * f)
	{
	    for (std::vector<std::shared_ptr<Cell>> & row : m_puzzleRows)
	    {
	        for (std::shared_ptr<Cell> cell : row)
	            (*f)(cell);
	    }
	}

    void forEachTraversalDirection (std::function<void(Direction d)> * f);

    Coordinate findPipeEnd (PipeId id, PipeEnd end) const noexcept(false)
    { return m_def.findPipeEnd(id, end); }

    unsigned getNumPipes () const noexcept
    { return m_def.getPipeIds().size(); }

    unsigned getNumRows () const noexcept
    { return m_def.getNumRows(); }

    unsigned getNumCols () const noexcept
    { return m_def.getNumCols(); }

    bool isEndpoint (Coordinate c) const noexcept
    { return m_def.isEndpoint(c); }

    bool passCoordinateRangeCheck (Coordinate c) const noexcept
    { return m_def.passCoordinateRangeCheck(c); }

  private:
    Puzzle (const PuzzleDefinition & def);

    /** The puzzle definition that generated this puzzle. */
    const PuzzleDefinition & m_def;

	std::vector<PuzzleRow> m_puzzleRows;

    std::set<Coordinate> m_injectedRoute;

  //friend class Plumber; // To tinker with the pipes that are private
  friend class PuzzleDefinition; // To be able to create puzzle via private constructor
};

#endif
