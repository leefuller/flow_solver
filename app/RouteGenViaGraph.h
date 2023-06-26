#ifndef ROUTEGENVIAGRAPH_H
#define ROUTEGENVIAGRAPH_H

#include <map>

#include "RouteGenerator.h"
#include "Puzzle.h"
#include "Matrix.h"
#include "Graph.h"

/**
 * Uses a Graph data structure to represent the possible routes.
 */
class RouteGenViaGraph : public RouteGenerator
{
  public:
    RouteGenViaGraph (ConstPuzzlePtr puzzle);

    virtual void generateRoutes (PipeId id, ConstPuzzlePtr puzzle) override;

    void setPathValidator (std::function<bool(const std::vector<ConstCellPtr>&)> * callback) noexcept
    { m_graph.setValidatePathCallback(callback); }

  private:
    void createGraph (const Puzzle & puzzle, PipeId idPipe);

    void traverseToCreateGraph (const Puzzle & puzzle, PipeId idPipe, Coordinate from, Matrix<bool> & visited);

    void handleStartEndPoint (const Puzzle & puzzle, ConstCellPtr pCell, Matrix<bool> & visited);

    bool receivePath (Graph<ConstCellPtr>::Path & path);

    void validatePath (const Graph<ConstCellPtr>::Path & path);

    Graph<ConstCellPtr> m_graph;

    ConstPuzzlePtr m_puzzle;

    Matrix<bool> m_visited;

    std::function<bool(Graph<ConstCellPtr>::Path&)> m_pathReceiver;
};

#endif
