#ifndef SOLVER_H
#define SOLVER_H

#include <vector>
#include <set>
#include <functional>
#include <memory>
#include <iostream>

#include "RouteGenViaGraph.h"
#include "../include/Puzzle.h"
#include "../include/PuzzleDef.h"
#include "RouteReceiver.h"

class Solver : public RouteReceiver
{
  public:
    Solver (const char * puzzleDef);
	Solver (const PuzzlePtr p, const std::set<PipeId> & pipeIds);

	Solver (const Solver &) = delete;

    bool solve();

	virtual bool processRoute (PipeId idPipe, Route & route) override;

	bool checkSolution (std::vector<std::pair<PipeId, Route>>::iterator start, std::vector<std::pair<PipeId, Route>>::iterator end);

  private:
	void generateRoutes (PipeId idPipe);

	void addRoute (PipeId idPipe, const Route & route);

	void addPipeIdToIdSetIfCellIsStart (ConstCellPtr cell) noexcept;

	void connectAndRevise (CellPtr cellFrom, CellPtr cellAdjacent, CellConnection con);

	std::set<CellPtr> reviseCell (CellPtr pCell) noexcept(false);

	void checkCornerFormations (CellPtr p, bool & changed);
	void checkCornerFormation (CellPtr pCell, Direction dCorner);
	void checkFillToObstruction (CellPtr pCell, bool & changed);
	void checkOneWay (CellPtr pCellFrom, bool & changed);

	bool checkObstructionAfter1 (ConstCellPtr pCell, Direction d);

	void connectIfOnlyOnePossibility (CellPtr p, bool & changed);

	void updateRemovePossibleForAllOther (PipeId idPipe) noexcept;
	bool isRouteComplete (PipeId idPipe) const noexcept;

	/**
	 * Check whether a cell is a vertical channel, defined by the walls.
	 * Disregards pipes.
	 * @param coord     Coordinate of cell to check
	 * @return true if the cell at the given coordinate has 2 parallel vertical walls
	 */
	bool isVerticalChannel (Coordinate coord) const noexcept
	{ return m_puzzle->getConstCellAtCoordinate(coord)->isVerticalChannel(); }

	/**
	 * Check whether a cell is a horizontal channel, defined by the walls.
	 * Disregards pipes.
	 * @param coord     Coordinate of cell to check
	 * @return true if the cell at the given coordinate has 2 parallel horizontal walls
	 */
	bool isHorizontalChannel (Coordinate coord) const noexcept
	{ return m_puzzle->getConstCellAtCoordinate(coord)->isHorizontalChannel(); }

    bool isAdjacentToChannelOpening (Coordinate coord) const noexcept;

    bool validatePath (const std::vector<ConstCellPtr> & path);

	void setSolved (bool solved = true) noexcept
	{ m_solved = solved; }

	bool isSolved () const noexcept
	{ return m_solved; }

    /** Definition for puzzle to be solved */
    PuzzleDefinition m_puzzleDef;

    /** Puzzle to be solved */
    PuzzlePtr m_puzzle;

    /** Pipe identifiers from puzzle */
    std::set<PipeId> m_pipeIds;

    RouteGenViaGraph m_routeGen;

    std::map<PipeId, Route> m_prelimRoutes;

	/** Each pipe has a list of possible routes. */
	std::map<PipeId, std::vector<Route>> m_routesDict;

	//std::vector<std::pair<PipeId, Route>> m_routeList;

	bool m_solved{false};

	friend void checkSolution (Solver & solver, std::vector<std::pair<PipeId, Route>>::iterator start, std::vector<std::pair<PipeId, Route>>::iterator end);
};

/**
 * Use to inject a route into a puzzle.
 * Automatically removes it when this is destroyed.
 */
class TryRoute
{
    public:
        TryRoute (PuzzlePtr puzzle, PipeId idPipe, const Route & route)
            : m_puzzle(puzzle), m_route(route)
        {
            m_puzzle->insertRoute(idPipe, route);
        }

        ~TryRoute ()
        {
            m_puzzle->removeRoute();
        }

    private:
        PuzzlePtr m_puzzle;
        Route m_route;
};

#endif
