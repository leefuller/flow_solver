#include "Puzzle.h"
#include "RouteGenViaGraph.h"
#include "Cell.h"
#include "Logger.h"

static Logger & logger = Logger::getDefaultLogger();

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

void RouteGenViaGraph::receivePath (Graph<ConstCellPtr>::Path & path)
{
    // The emission from the graph does not identify anything other than the path between nodes.
    // Need to convert path to Route and emit
    if (path.empty())
        return;
    PipeId idPipe = path[0]->getPipeId();
    if (idPipe == NO_PIPE_ID) // should not happen
        return;
    Route route;
    for (ConstCellPtr pCell : path)
        route.push_back(pCell->getCoordinate());
    // If path end is proxy to route endpoint, append the cells to the endpoint
    // to complete the route
    if (!path.back()->isEndpoint())
    {
        auto it = std::end(route);
        auto pv = std::prev(it); // point at last coordinate in route
        if (pv != std::begin(route))
            pv = std::prev(pv); // point at coordinate before last in route
        // Determine direction of fixture connection into last cell in the path,
        // so that direction is not used as path to follow
        Direction incomingDir = getDirectionBetweenCoordinates(*pv, route.back());

        // Follow connections to endpoint
        ConstCellPtr pCell = path.back();
        // There should be a fixture connection at the intermediary point.
        while (!pCell->isEndpoint())
        {
            for (Direction d : allTraversalDirections)
            {
                if (d == opposite(incomingDir))
                    continue;
                if (pCell->getConnection(d) == CellConnection::FIXTURE_CONNECTION)
                {
                    pCell = m_puzzle->getConstCellAdjacent(pCell->getCoordinate(), d);
                    route.push_back(pCell->getCoordinate());
                    incomingDir = d;
                    break;
                }
            }
        }
    }
    RouteGenerator::emitRoute(idPipe, route);

}

RouteGenViaGraph::RouteGenViaGraph (ConstPuzzlePtr puzzle)
    : m_visited(puzzle->getNumRows(), puzzle->getNumCols()),
      m_pathReceiver(std::bind(&RouteGenViaGraph::receivePath, std::reference_wrapper<RouteGenViaGraph>(*this), std::placeholders::_1))
{
    m_visited.setAllValues(false);
    m_graph.setEmitPathCallback(&m_pathReceiver);
}

/**
 * Generate all routes between start and end points for pipe.
 */
void RouteGenViaGraph::generateRoutes (PipeId id, ConstPuzzlePtr puzzle)
{
    m_puzzle = puzzle;

    createGraph(*puzzle, id);

    /*GraphOutputter<ConstCellPtr> out(std::cout);
    m_graph.accept(out);*/

    Coordinate start = puzzle->findPipeEnd(id, PipeEnd::PIPE_START);
    // If preliminary logic has derived some fixtures attached to the PIPE_END,
    // then the path generator needs to connect to the start of that, rather than PIPE_END.
    // Similar is not necessary for start, because the route generation
    // implicitly follows connections from the start.
    Coordinate end = puzzle->findPipeEnd(id, PipeEnd::PIPE_END);
    Direction incoming = Direction::NONE;
    ConstCellPtr destCell = puzzle->getConstCellAtCoordinate(end);
    do
    {
        for (Direction d : allTraversalDirections)
        {
            if (incoming == d)
                continue;
            destCell = puzzle->getConstCellAtCoordinate(end);
            if (destCell->getConnection(d) == CellConnection::FIXTURE_CONNECTION)
            {
                destCell = puzzle->getConstCellAdjacent(destCell->getCoordinate(), d);
                end = destCell->getCoordinate();
                incoming = opposite(d);
                break;
            }
        }
    }
    while (destCell->countFixtureConnections() > 1);

    ConstCellPtr startCell = puzzle->getConstCellAtCoordinate(start);
    //ConstCellPtr destCell = puzzle->getConstCellAtCoordinate(end);
    logger << "Generate paths from " << start << " to " << end << std::endl;
    m_graph.genAllPaths(startCell, destCell);
}

/**
 *
 */
void RouteGenViaGraph::handleStartEndPoint (const Puzzle & puzzle, ConstCellPtr pCell, Matrix<bool> & visited)
{
    // Check for a fixed connection.
    // If a fixed connection is found, the start/end point only needs 1 directed edge
    for (Direction direction : puzzle.getConnectedDirections(pCell->getCoordinate()))
    {
        if (pCell->getConnection(direction) == CellConnection::FIXTURE_CONNECTION)
        {
            ConstCellPtr pCellAdjacent = puzzle.getConstCellAdjacent(pCell->getCoordinate(), direction);
            if (pCellAdjacent == nullptr)
                continue;
            //if (pCellAdjacent->getPipeId() != pCell->getPipeId())
                //continue;
#if SUPPORT_ROUTE_DIRECTION
            if (pCell->getEndpoint() == PipeEnd::PIPE_START)
                m_graph.addDirectedEdge(pCell, pCellAdjacent); // out from start point
            else
                m_graph.addDirectedEdge(pCellAdjacent, pCell); // into endpoint
#else
            m_graph.addEdge(pCell, pCellAdjacent);
#endif
            return; // There can be only one fixture edge on start/end point
        }
    }
    // else start/end point has no attached fixtures, so there can be more than one edge
    for (Direction direction : puzzle.getConnectedDirections(pCell->getCoordinate()))
    {
        if (pCell->getConnection(direction) == CellConnection::NO_CONNECTOR)
            continue;

        ConstCellPtr pCellAdjacent = puzzle.getConstCellAdjacent(pCell->getCoordinate(), direction);
        if (pCellAdjacent == nullptr)
            continue;
        //if (pCellAdjacent->getPipeId() != pCell->getPipeId())
            //continue;
#if SUPPORT_ROUTE_DIRECTION
        if (pCell->getEndpoint() == PipeEnd::PIPE_START)
            m_graph.addDirectedEdge(pCell, pCellAdjacent); // out from start point
        else
            m_graph.addDirectedEdge(pCellAdjacent, pCell); // into endpoint
#else
            m_graph.addEdge(pCell, pCellAdjacent);
#endif
    }
}

/**
 * Recursive function used to create graph for pipe.
 */
void RouteGenViaGraph::traverseToCreateGraph (const Puzzle & puzzle, PipeId idPipe, Coordinate from, Matrix<bool> & visited)
{
    if (visited.at(from))
        return;
    // Cell for current graph node
    ConstCellPtr pCell = puzzle.getConstCellAtCoordinate(from);
    visited.at(from) = true;
    if (pCell->getPipeId() != idPipe && pCell->getPipeId() != NO_PIPE_ID)
        return; // cell is for different pipe

    if (pCell->getEndpoint() == PipeEnd::PIPE_START || pCell->getEndpoint() == PipeEnd::PIPE_END)
    {
        handleStartEndPoint(puzzle, pCell, visited);
    }
    else
    {
        for (Direction direction : puzzle.getConnectedDirections(pCell->getCoordinate()))
        {
            if (pCell->getConnection(direction) == CellConnection::NO_CONNECTOR)
                continue;

            // Cell adjacent in direction
            ConstCellPtr pCellAdjacent = puzzle.getConstCellAdjacent(pCell->getCoordinate(), direction);
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
                //if (pCellAdjacent->hasPossible(idPipe))
                    m_graph.addEdge(pCell, pCellAdjacent);
            }
        }
    }

    // Recurse for each neighbour
    for (Direction direction : allTraversalDirections)
    {
        ConstCellPtr pCellAdjacent = puzzle.getConstCellAdjacent(pCell->getCoordinate(), direction);
        if (pCellAdjacent != nullptr)
        {
            if (pCellAdjacent->getPipeId() == idPipe || pCellAdjacent->getPipeId() == NO_PIPE_ID)
                traverseToCreateGraph(puzzle, idPipe, pCellAdjacent->getCoordinate(), visited);
        }
    }
}

void RouteGenViaGraph::createGraph (const Puzzle & puzzle, PipeId idPipe)
{
    Coordinate start = puzzle.findPipeEnd(idPipe, PipeEnd::PIPE_START);
    m_graph.clear();
    m_visited.setAllValues(false);
    traverseToCreateGraph(puzzle, idPipe, start, m_visited);
}
