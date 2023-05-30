#ifndef SOLVER_H
#define SOLVER_H

#include <vector>
#include <set>
#include <functional>
#include <memory>
#include <iostream>

#include "Graph.h"
#include "../include/Puzzle.h"
#include "../include/PuzzleDef.h"
#include "RouteReceiver.h"
#include "Helper.h"

class Solver : public RouteReceiver
{
  public:
    Solver (const char * puzzleDef);

	Solver (const Solver &) = delete;

    bool solve();

	virtual void processRoute (PipeId idPipe, Route & route);

	bool checkSolution (std::vector<std::pair<PipeId, Route>>::iterator start, std::vector<std::pair<PipeId, Route>>::iterator end);

  private:
	void generateRoutes (const std::map<PipeId, Route> & existing);

	void addRoute (PipeId idPipe, const Route & route);

	void addPipeIdToIdSetIfCellIsStart (ConstCellPtr cell) noexcept;

	void connectAndRevise (CellPtr cellFrom, CellPtr cellAdjacent, CellConnection con);

	std::set<CellPtr> reviseCell (CellPtr pCell) noexcept(false);

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

    /** Definition for puzzle to be solved */
    PuzzleDefinition m_puzzleDef;

    /** Puzzle to be solved */
    PuzzlePtr m_puzzle;

    /** Pipe identifiers from puzzle */
    std::set<PipeId> m_pipeIds;

    RouteGenViaGraph m_routeGen;

	/** Each pipe has a list of possible routes. */
	std::map<PipeId, std::vector<Route>> m_routesDict;

	std::vector<std::pair<PipeId, Route>> m_routeList;

	friend void checkSolution (Solver & solver, std::vector<std::pair<PipeId, Route>>::iterator start, std::vector<std::pair<PipeId, Route>>::iterator end);
};

#endif
