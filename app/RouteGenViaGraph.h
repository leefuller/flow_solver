#ifndef ROUTEGENVIAGRAPH_H
#define ROUTEGENVIAGRAPH_H

#include <map>
#include <memory> // unique_ptr, shared_ptr

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
    RouteGenViaGraph (std::shared_ptr<const Puzzle> puzzle);

    virtual void generateRoutes (PipeId id, std::shared_ptr<const Puzzle> puzzle) override;

  private:
    void createGraph (const Puzzle & puzzle, PipeId idPipe);

    void traverseToCreateGraph (const Puzzle & puzzle, PipeId idPipe, Coordinate from, Matrix<bool> & visited);

    void handleStartEndPoint (const Puzzle & puzzle, std::shared_ptr<const Cell> pCell, Matrix<bool> & visited);

    void receivePath (Graph<std::shared_ptr<const Cell>>::Path & path);

    Graph<std::shared_ptr<const Cell>> m_graph;

    Matrix<bool> m_visited;

    std::function<void(Graph<std::shared_ptr<const Cell>>::Path&)> m_pathReceiver;
};

#endif
