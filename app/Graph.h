#ifndef INCLUDE_GRAPH_H
#define INCLUDE_GRAPH_H

#include <list>
#include <map>
#include <set>
#include <deque>
#include <stack>
#include <functional>
#include <optional>
#include <string>
//#include <iostream>

/*
 Graph Terminology:
   Vertex:         Each node of the graph is called a vertex.
   Edge:           The link or path between two vertices is called an edge. It connects two or more vertices.
   Adjacent node:  If two nodes are connected by an edge then they are called adjacent nodes or neighbours.
   Degree of the node: The number of edges that are connected to a particular node.
   Path:           The sequence of nodes we need to follow to travel from one vertex to another.
   Closed path:    If the initial node is the same as a terminal node, then that path is termed as the closed path.
   Simple path:    A closed path in which all the other nodes are distinct is called a simple path.
   Cycle:          A path in which there are no repeated edges or vertices and the first and last vertices are the same.
   Connected Graph: A connected graph is one in which there is a path between each of the vertices.
                    This means that there is not a single vertex which is isolated or without a connecting edge.
   Complete Graph:  A graph in which each node is connected to another.
                    If N is the total number of nodes in a graph then the complete graph contains N(N-1)/2 number of edges.
   Weighted graph: A positive value assigned to each edge is called weight.
                   A graph containing weighted edges is called a weighted graph.
                   The weight of an edge e is denoted by w(e) and it indicates the cost of traversing an edge.
   Diagraph:       A digraph is a graph in which every edge is associated with a specific direction,
                   and the traversal can be done in specified direction only.

 A graph can be stored as a sequential representation or as a linked representation.
  - In the sequential representation of graphs, we use the adjacency matrix.
    An adjacency matrix is a matrix of size n x n where n is the number of vertices in the graph.

  - We use the adjacency list for the linked representation of the graph.
    The adjacency list representation maintains each node of the graph and a link to the nodes that are adjacent to this node.
    When we traverse all the adjacent nodes, we set the next pointer to null at the end of the list.
 */

/**
 * Graph implementation.
 * NodeT is the type of object held by nodes.
 *
 * Note std::shared_ptr comparison compares pointer values. So it is ok for NodeT to be a shared_ptr.
 */
template<typename NodeT>
class Graph
{
  public:
    using Path = std::vector<NodeT>;

    /**
     * Add edge between nodes.
     * @param node1     Node at one end of edge
     * @param node2     Node at one end of edge
     * @param directed  If true, the edge is a directed edge from node1 to node2. Otherwise edge is traversable both ways.
     */
    void addEdge (NodeT node1, NodeT node2, bool directed = false) noexcept
    {
        m_adjList[node1].insert(node2);
        if (!directed)
        {
            // Add reverse direction for undirected edge
            m_adjList[node2].insert(node1);
        }
        else // directed
        {
            // Add empty adjacency list for node2, if not existing.
            if (m_adjList.find(node2) == m_adjList.end())
                m_adjList[node2] = std::set<NodeT>();
        }
    }

    /**
     * Add directed edge between nodes.
     * @param nodeS     Source node
     * @param nodeD     Destination node
     */
    void addDirectedEdge (NodeT nodeS, NodeT nodeD)
    { addEdge(nodeS, nodeD, true); }

    /*void removeEdge (NodeT node1, NodeT node2)
    {
        auto found1 = m_adjList.find(node1);
        if (found1 != m_adjList.end())
            found1->second.erase(node2);
        auto found2 = m_adjList.find(node2);
        if (found2 != m_adjList.end())
            found2->second.erase(node1);
    }*/

    /** @return true if there are no nodes in the graph */
    bool isEmpty () const noexcept
    { return m_adjList.empty(); }

    /** Erases content of map. If nodes were pointers, the item they pointed at is not modified. */
    void clear () noexcept
    { m_adjList.clear(); }

    class GraphException : public std::exception
    {
        public:
            GraphException (const std::string & msg) noexcept
                : std::exception(), m_msg(msg)
            {}

            GraphException () noexcept
                : GraphException("graph exception")
            {}

            virtual const char * what () const noexcept override
            { return m_msg.c_str(); }

        private:
            std::string m_msg;
    };

    /**
     * Discover all paths between source and destination.
     * Any, and all paths discovered cause the emitPath function to be called.
     * @param source    Source node
     * @param dest      Destination node
     */
    //void genAllPaths (NodeT source, NodeT dest, std::vector<NodeT> & path)
    void genAllPaths (NodeT source, NodeT dest) noexcept(false)
    {
        // Check source exists
        auto lam = [source](NodeT& node) { return node == source; };
        NodeT node = breadthFirstSearch(lam);
        // Check dest exists
        breadthFirstSearch([dest](NodeT& node) { return node == dest; });

        std::set<NodeT> visited;
        std::vector<NodeT> path; // FIXME: This could get fairly big
        m_invalid.clear();
        genAllPathsHelper(node, dest, visited, path);
    }

    /**
     * Perform a depth-first search for a node where a check function returns true.
     * @param checkFn   function to identify (by returning true) if the required node is found.
     * @return Node, if found
     * @throw GraphException if no match found
     */
    NodeT depthFirstSearch (std::function<bool(NodeT&)> checkFn) noexcept(false)
    {
        if (isEmpty())
            throw GraphException("search attempt on empty graph");
            //return nullptr;

        std::deque<NodeT> q;
        std::stack s(q);
        std::set<NodeT> visited;

        // Push each first level node onto stack in order to support a divided graph,
        // having nodes which are not reachable from across the divide.
        // For a fully connected graph, they will all be visited before completion of DFS from the first, anyway.
        for (auto mapIt = std::begin(m_adjList); mapIt != std::end(m_adjList); ++mapIt)
        {
            s.push(mapIt->first);
        }
        while (!s.empty())
        {
            NodeT node = s.top();
            s.pop();
            // Recursive search
            std::optional<NodeT> found;
            rDFS(node, visited, checkFn, found);
            if (found.has_value())
                return found.value();
        }
        throw GraphException("Node not found in depth first search");
    }

    /**
     * Perform a breadth-first search for a node where a check function returns true.
     * @param checkFn   function to identify (by returning true) if the required node is found.
     * @return Node, if found
     * @throw GraphException if no match found
     */
    NodeT breadthFirstSearch (std::function<bool(NodeT&)> checkFn) noexcept(false)
    {
        if (isEmpty())
            throw GraphException("search attempt on empty graph");
            //return nullptr;

        std::deque<NodeT> q;
        std::set<NodeT> visited;

        // Find a starting node
        for (auto mapIt = std::begin(m_adjList); mapIt != std::end(m_adjList); ++mapIt)
        {
            q.push_back(mapIt->first);
            // No break in order to support a divided graph,
            // having nodes which are not reachable from across the divide.
            // For a true BFS on a fully connected graph, the break is necessary here.
            //break;
        }

        while (!q.empty())
        {
            // Get next queued node
            NodeT node = q.front();

            // Check if this is the node searched for.
            if (checkFn(node))
                return node;

            // Mark node visited
            visited.insert(node);
            // Put non-visited neighbours into queue
            std::set<NodeT> & neighbours = getAdjacentNodes(node);
            for (NodeT neighbour : neighbours)
            {
                if (visited.find(neighbour) == visited.end())
                    q.push_back(neighbour);
            }

            q.pop_front(); // Done with current node
        }
        throw GraphException("Node not found in breadth first search");
        //return nullptr;
    }

    class Visitor
    {
        public:
            virtual void visitNode (const NodeT & node, std::set<NodeT> adjacent) noexcept = 0;
    };

    /**
     * Allow a Visitor to visit each node.
     */
    void accept (Graph::Visitor & visitor)
    {
        for (auto s = std::begin(m_adjList); s != std::end(m_adjList); ++s)
            visitor.visitNode(s->first, s->second);
    }

    void setEmitPathCallback (std::function<void(Path&)> * callback) noexcept
    { emitPathCallback = callback; }

    void setValidatePathCallback (std::function<bool(const Path&)> * callback) noexcept
    { validatePathCallback = callback; }

  private:

    std::set<NodeT> & getAdjacentNodes (NodeT node) noexcept(false)
    {
        auto found = m_adjList.find(node);
        if (found == m_adjList.end())
            throw GraphException("node not found adjacent");
        return m_adjList.at(node);
    }

    /**
     * Callback from genAllPaths when a path is found.
     * This only occurs during genAllPaths()
     */
    void emitPath (Path & path) noexcept
    {
        if (emitPathCallback != nullptr)
        {
            try
            {
                (*emitPathCallback)(path);
            }
            catch (...)
            {} // swallow anything thrown by callback
        }
    }

    /**
     * Recursive part of depth-first search for a node where a check function returns true.
     */
    bool rDFS (NodeT & node, std::set<NodeT> & visited, std::function<bool(NodeT&)> checkFn, std::optional<NodeT> & opt) noexcept(false)
    {
        // Check if this is the node searched for.
        if (checkFn(node))
        {
            opt = node;
            return true;
        }
        visited.insert(node);
        // Recurse for non-visited neighbours
        std::set<NodeT> & neighbours = getAdjacentNodes(node);
        for (NodeT neighbour : neighbours)
        {
            if (visited.find(neighbour) == visited.end())
            {
                if (rDFS(neighbour, visited, checkFn, opt))
                    return true;
            }
        }
        return false;
    }

    /**
     * Recursive part of path generation.
     */
    void genAllPathsHelper (NodeT pos, NodeT dest, std::set<NodeT> & visited, Path & path) noexcept(false)
    {
        visited.insert(pos);
        path.push_back(pos);
        //std::cout << "Push node " << pos << std::endl;
        if (pos == dest)
        {
            //std::cout << "emit path" << std::endl;
            emitPath(path);
        }
        else
        {
            if (validatePathCallback != nullptr)
            {
                if (!(*validatePathCallback)(path))
                {
                    // No need to change path here. Rolling back the recursion does that.
                    //std::cout << "Add node " << pos << " to invalid set" << std::endl;
                    m_invalid.insert(pos);
                    //std::cout << "Pop node " << path.back() << std::endl;
                    path.pop_back();
                    //visited.erase(pos);
                    return;
                }
            }
            // Not at destination yet. Recur for adjacent nodes not visited yet.
            const std::set<NodeT> & adjacent = getAdjacentNodes(pos);
            for (const NodeT & node : adjacent)
            {
                if (m_invalid.find(node) != m_invalid.end()) // node is in the invalid group
                {
                    //std::cout << "skip invalid node " << node << std::endl;
                    continue;
                }
                if (visited.find(node) == visited.end())
                    genAllPathsHelper(node, dest, visited, path);
            }
            // will now go back in the recursion stack,
            // so adjacent nodes in invalid group are no longer invalid
            // because they could be valid in another path.
            for (const NodeT & node : adjacent)
            {
                if (m_invalid.find(node) != m_invalid.end())
                {
                    //std::cout << "Remove node " << node << " from invalid set" << std::endl;
                    m_invalid.erase(node);
                }
            }
        }
        // Remove current node from path and mark it not visited
        //std::cout << "Pop node " << path.back() << std::endl;
        path.pop_back();
        visited.erase(pos);
    }

    /** Adjacency list */
    std::map<NodeT, std::set<NodeT>> m_adjList;

    std::set<NodeT> m_invalid;

    /**
     * Callback from genAllPaths to validate path whenever a node is added.
     * Provides a means to prevent generating paths that would be invalid
     * due to something else.
     */
    std::function<bool(const Path&)> * validatePathCallback{nullptr};

    /**
     * Callback from genAllPaths when a path is found.
     */
    std::function<void(Path&)> * emitPathCallback{nullptr};
};

#endif
