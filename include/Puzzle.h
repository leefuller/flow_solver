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

class Puzzle;
using PuzzlePtr = std::shared_ptr<Puzzle>;
using ConstPuzzlePtr = std::shared_ptr<const Puzzle>;

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

	/** Output to stream in matrix format. */
	std::ostream & streamPuzzleMatrix (std::ostream & os) const noexcept
	{
	    for (const PuzzleRow & row : m_puzzleRows)
	        os << row;
	    return os;
	}

	/**
	 * Determine whether a coordinate change is valid, given starting coordinate and direction.
	 * Disregards walls.
	 * @param coord     Starting coordinate
	 * @param adj       Direction
	 * @return true if coordinate change would be valid
	 */
	bool isCoordinateChangeValid (Coordinate from, Direction a) const noexcept
	{ return m_def.isCoordinateChangeValid(from, a); }

    static bool checkIfSolution (ConstPuzzlePtr puzzle, const std::map<PipeId, Route> & s);

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

    CellPtr getCellAtCoordinate(Coordinate c) noexcept;
    ConstCellPtr getConstCellAtCoordinate(Coordinate c) const noexcept;

	CellPtr getCellAdjacent (Coordinate c, Direction d) ;//noexcept;
	ConstCellPtr getConstCellAdjacent (Coordinate c, Direction d) const ;//noexcept;

	std::map<Direction, ConstCellPtr> getSurroundingCells (Coordinate c) const ;//noexcept;

	std::map<Direction, ConstCellPtr> getAdjacentCellsInTraversalDirections (Coordinate coord, bool wallsBlock = false) const ;//noexcept

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

    void forEveryCellMutable (std::function<void(CellPtr)> * f)
	{
	    for (PuzzleRow & row : m_puzzleRows)
	    {
	        for (CellPtr cell : row)
	            (*f)(cell);
	    }
	}

    void forEveryCell (std::function<void(ConstCellPtr)> * f) const
    {
        for (const PuzzleRow & row : m_puzzleRows)
        {
            for (CellPtr cell : row)
                (*f)(cell);
        }
    }

    void forEachTraversalDirection (std::function<void(Direction d)> * f);

    /**
     * Find a particular endpoint
     * @param id    Pipe identifier
     * @param end   End identifier. (ie. find which end)
     * @return coordinate where end of pipe is found
     * @throw exception if not found
     */
    Coordinate findPipeEnd (PipeId id, PipeEnd end) const noexcept(false)
    { return m_def.findPipeEnd(id, end); }

    unsigned getNumPipes () const noexcept
    { return m_def.getPipeIds().size(); }

    unsigned getNumRows () const noexcept
    { return m_def.getNumRows(); }

    unsigned getNumCols () const noexcept
    { return m_def.getNumCols(); }

    /** @return true if cell at coordinate is any endpoint */
    bool isEndpoint (Coordinate c) const noexcept
    { return m_def.isEndpoint(c); }

    bool passCoordinateRangeCheck (Coordinate c) const noexcept
    { return m_def.passCoordinateRangeCheck(c); }

    unsigned gapToObstruction (Coordinate c, Direction d) const noexcept;
    std::array<unsigned, 4> getGapsToObstructions (Coordinate c) const noexcept;

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
