#include "Puzzle.h"
#include "RouteGenViaGraph.h"
#include "Cell.h"

/*
 * Using graph terminology:
 * - Each cell is a node/vertex.
 * - Edges between cells exist implicitly in the puzzle via whether it is possible to traverse between the cells
 *   (as declared in the puzzle definition)
 * - The graph is undirected.
 *
 * Cells can be adjacent in any of 4 directions. ie. Every cell has no more than 4 edges connected.
 * The puzzle data structure parsed from the puzzle definition implicitly provides the graph.
 */

template<typename NodeT>
class GraphOutputter : public Graph<NodeT>::Visitor
{
    public:
        GraphOutputter (std::ostream & os)
            : m_os(os)
        {}

        GraphOutputter (const GraphOutputter<NodeT>&) = default;
        GraphOutputter (GraphOutputter<NodeT>&&) = default;
        GraphOutputter & operator= (const GraphOutputter<NodeT>&) = default;
        GraphOutputter & operator= (GraphOutputter<NodeT>&&) = default;

        virtual void visitNode (const NodeT & node, std::set<NodeT> adjacent) noexcept override
        {
            m_os << node->getCoordinate() << ":";
            for (const NodeT& n : adjacent)
                m_os << n->getCoordinate();
            m_os << std::endl;
        }

    private:
        std::ostream & m_os;
};

void RouteGenViaGraph::receivePath (Graph<std::shared_ptr<const Cell>>::Path & path)
{
    // The emission from the graph does not identify anything other than the path between nodes.
    // Need to convert path to Route and emit
    if (path.empty())
        return;
    PipeId idPipe = path[0]->getPipeId();
    if (idPipe == NO_PIPE_ID) // should not happen
        return;
    Route route;
    for (std::shared_ptr<const Cell> pCell : path)
        route.push_back(pCell->getCoordinate());
    RouteGenerator::emitRoute(idPipe, route);

}

RouteGenViaGraph::RouteGenViaGraph (std::shared_ptr<const Puzzle> puzzle)
    : m_visited(puzzle->getNumRows(), puzzle->getNumCols()),
      m_pathReceiver(std::bind(&RouteGenViaGraph::receivePath, std::reference_wrapper<RouteGenViaGraph>(*this), std::placeholders::_1))
{
    m_visited.setAllValues(false);
    m_graph.setEmitPathCallback(&m_pathReceiver);
}

/**
 * 
 */
//void RouteGenViaGraph::generateRoutes (PipeId id, std::shared_ptr<const Puzzle> puzzle, const Route & existing)
void RouteGenViaGraph::generateRoutes (PipeId id, std::shared_ptr<const Puzzle> puzzle)
{
    createGraph(*puzzle, id); // No need to use existing routes parameter, because puzzle should now reflect those.

    Coordinate start = puzzle->findPipeEnd(id, PipeEnd::PIPE_END_1);
    Coordinate end = puzzle->findPipeEnd(id, PipeEnd::PIPE_END_2);

    Route route;
    std::shared_ptr<const Cell> startCell = puzzle->getConstCellAtCoordinate(start);
    std::shared_ptr<const Cell> destCell = puzzle->getConstCellAtCoordinate(end);
    m_graph.genAllPaths(startCell, destCell);
}

/**
 *
 */
void RouteGenViaGraph::handleStartEndPoint (const Puzzle & puzzle, std::shared_ptr<const Cell> pCell, Matrix<bool> & visited)
{
    // Check for a fixed connection.
    // If a fixed connection is found, the start/end point only needs 1 directed edge
    for (Direction direction : puzzle.getConnectedDirections(pCell->getCoordinate()))
    {
        if (pCell->getConnection(direction) == CellConnection::FIXTURE_CONNECTION)
        {
            std::shared_ptr<const Cell> pCellAdjacent = puzzle.getConstCellAdjacent(pCell->getCoordinate(), direction);
            if (pCellAdjacent == nullptr)
                continue;
            //if (pCellAdjacent->getPipeId() != pCell->getPipeId())
                //continue;
            if (pCell->getEndpoint() == PipeEnd::PIPE_END_1)
                m_graph.addDirectedEdge(pCell, pCellAdjacent); // out from start point
            else
                m_graph.addDirectedEdge(pCellAdjacent, pCell); // into endpoint
            return; // There can be only one fixture edge on start/end point
        }
    }
    // else start/end point has no attached fixtures, so there can be more than one edge
    for (Direction direction : puzzle.getConnectedDirections(pCell->getCoordinate()))
    {
        if (pCell->getConnection(direction) == CellConnection::NO_CONNECTOR)
            continue;

        std::shared_ptr<const Cell> pCellAdjacent = puzzle.getConstCellAdjacent(pCell->getCoordinate(), direction);
        if (pCellAdjacent == nullptr)
            continue;
        //if (pCellAdjacent->getPipeId() != pCell->getPipeId())
            //continue;
        if (pCell->getEndpoint() == PipeEnd::PIPE_END_1)
            m_graph.addDirectedEdge(pCell, pCellAdjacent); // out from start point
        else
            m_graph.addDirectedEdge(pCellAdjacent, pCell); // into endpoint
    }
}

void RouteGenViaGraph::traverseToCreateGraph (const Puzzle & puzzle, PipeId idPipe, Coordinate from, Matrix<bool> & visited)
{
    if (visited.at(from))
        return;
    // Cell for current graph node
    std::shared_ptr<const Cell> pCell = puzzle.getConstCellAtCoordinate(from);
    visited.at(from) = true;
    if (pCell->getPipeId() != idPipe && pCell->getPipeId() != NO_PIPE_ID)
        return; // cell is for different pipe

    if (pCell->getEndpoint() == PipeEnd::PIPE_END_1 || pCell->getEndpoint() == PipeEnd::PIPE_END_2)
    {
        handleStartEndPoint(puzzle, pCell, visited);
    }
    else // else "normal" cell can have 2 fixtures
    {
        if (pCell->countFixtureConnections() == 2)
        {
            // Only add edges and traverse for fixed connectors.
            // If other logic has worked properly, then the cell should have no other connectors.
            // So do nothing here. Loop below should iterate over only the 2 fixed connections.
        }
        for (Direction direction : puzzle.getConnectedDirections(pCell->getCoordinate()))
        {
            if (pCell->getConnection(direction) == CellConnection::NO_CONNECTOR)
                continue;

            // Cell adjacent in direction
            std::shared_ptr<const Cell> pCellAdjacent = puzzle.getConstCellAdjacent(pCell->getCoordinate(), direction);
            if (pCellAdjacent == nullptr)
                continue;
            if (pCellAdjacent->getPipeId() != NO_PIPE_ID && pCellAdjacent->getPipeId() != idPipe)
                continue; // Can be no connection in direction of different pipe
            if (visited.at(pCellAdjacent->getCoordinate()))
                continue; // An edge will have been added from the visited cell/node, if necessary.

            if (pCellAdjacent->getPipeId() == idPipe)
            {
                /*if (pCell->getConnection(direction) == CellConnection::FIXTURE_CONNECTION)
                {
                    // TODO there could be a directed edge because a route has 1 direction,
                    // except that we don't know the direction in cases where the route has not traced from start point.
                    m_graph.addDirectedEdge(pCell, pCellAdjacent); // include existing connection in graph
                }*/
                m_graph.addEdge(pCell, pCellAdjacent);
            }
            else if (pCellAdjacent->getPipeId() == NO_PIPE_ID)
            {
                m_graph.addEdge(pCell, pCellAdjacent);
            }
        }
    }

    // Recurse for each neighbour
    for (Direction direction : allTraversalDirections)
    {
        std::shared_ptr<const Cell> pCellAdjacent = puzzle.getConstCellAdjacent(pCell->getCoordinate(), direction);
        if (pCellAdjacent != nullptr)
        {
            if (pCellAdjacent->getPipeId() == idPipe || pCellAdjacent->getPipeId() == NO_PIPE_ID)
                traverseToCreateGraph(puzzle, idPipe, pCellAdjacent->getCoordinate(), visited);
        }
    }
}

void RouteGenViaGraph::createGraph (const Puzzle & puzzle, PipeId idPipe)
{
    Coordinate start = puzzle.findPipeEnd(idPipe, PipeEnd::PIPE_END_1);
    m_graph.clear();
    m_visited.setAllValues(false);
    traverseToCreateGraph(puzzle, idPipe, start, m_visited);
}
