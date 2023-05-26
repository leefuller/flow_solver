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

  private:
    void createGraph (const Puzzle & puzzle, PipeId idPipe);

    void traverseToCreateGraph (const Puzzle & puzzle, PipeId idPipe, Coordinate from, Matrix<bool> & visited);

    void handleStartEndPoint (const Puzzle & puzzle, ConstCellPtr pCell, Matrix<bool> & visited);

    void receivePath (Graph<ConstCellPtr>::Path & path);

    Graph<ConstCellPtr> m_graph;

    Matrix<bool> m_visited;

    std::function<void(Graph<ConstCellPtr>::Path&)> m_pathReceiver;
};

#endif
