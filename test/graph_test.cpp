//#include "GraphTest.h"
#include "../app/Graph.h"
#include "test_helper.h"

#include <iostream>
#include <memory>
#include <functional>
#include <algorithm>

class GraphOutputter : public Graph<int>::Visitor
{
    public:
        GraphOutputter (std::ostream & os)
            : m_os(os)
        {}

        GraphOutputter (const GraphOutputter&) = default;
        GraphOutputter (GraphOutputter&&) = default;
        GraphOutputter & operator= (const GraphOutputter&) = default;
        GraphOutputter & operator= (GraphOutputter&&) = default;

        virtual void visitNode (const int & node, std::set<int> adjacent) noexcept override
        {
            m_os << node << ":";
            for (const int & n : adjacent)
                m_os << ' ' << n;
            m_os << std::endl;
        }

    private:
        std::ostream & m_os;
};

static std::vector<std::vector<int>> paths;

void receivePath (std::vector<int> & path)
{
    std::cout << "Received path: ";
    for (auto it = std::begin(path); it != std::end(path); ++it)
    {
        std::cout << *it << ' ';
    }
    std::cout << std::endl;
    paths.push_back(path);
}

#define TEST_BFS    1
#define TEST_DFS    1
#define TEST_PATH_GEN   1

static bool testBFS (Graph<int> & graph, int lookup)
{
    //std::cout << "Run breadth first search for " << lookup << std::endl;
    try
    {
        return lookup = graph.breadthFirstSearch([lookup](int found) { return lookup == found; });
    }
    catch (Graph<int>::GraphException & ex)
    {
        std::cerr << ex.what() << std::endl;
    }
    return false;
}

static bool testDFS (Graph<int> & graph, int lookup)
{
    //std::cout << "Run depth first search for " << lookup << std::endl;
    try
    {
        return lookup == graph.depthFirstSearch([lookup](int found) { return lookup == found; });
    }
    catch (Graph<int>::GraphException & ex)
    {
        std::cerr << ex.what() << std::endl;
    }
    return false;
}

static bool testPathGen ()
{
    bool result = true;
    Graph<int> graph;

    graph.addEdge(3, 1);
    graph.addDirectedEdge(3, 6);
    graph.addEdge(2, 6);
    graph.addEdge(1, 2);
    graph.addEdge(4, 7); // <<<< Causes a divided graph, where not all nodes are reachable

    std::function<void(std::vector<int> & path)> fn = receivePath;
    graph.setEmitPathCallback(&fn);
    std::cout << "All paths from 1 to 6:" << std::endl;
    paths.clear();
    graph.genAllPaths(1, 6);
    std::cout << std::endl;
    std::vector<int> expect1 = {1, 2, 6};
    std::vector<int> expect2 = {1, 3, 6};
    result = result && check(paths.size() == 2);
    // TODO check
    //result = result && check(std::find(paths, expect1) != paths.end());
    //result = result && check(std::find(paths, expect2) != paths.end());

    std::cout << "All paths from 6 to 3:" << std::endl;
    expect1 = {6, 2, 1, 3};
    paths.clear();
    graph.genAllPaths(6, 3);
    result = result && check(paths.size() == 1);
    result = result && check(*paths.begin() == expect1);
    std::cout << std::endl;

    return result;
}

bool testGraph ()
{
    std::cout << "Graph test" << std::endl;
    bool result = true;
    try
    {
        Graph<int> graph;

        graph.addEdge(3, 1);
        graph.addDirectedEdge(3, 6);
        graph.addEdge(2, 6);
        graph.addEdge(1, 2);
        graph.addEdge(4, 7); // <<<< Causes a divided graph, where not all nodes are reachable
        GraphOutputter outputter(std::cout);
        graph.accept(outputter);

#if TEST_BFS
        result = result && check("Test BFS", testBFS(graph, 3));
        result = result && check("Test BFS", testBFS(graph, 7));
        result = result && check("Test BFS", !testBFS(graph, 5));
#endif
#if TEST_DFS
        result = result && check("Test DFS", testDFS(graph, 3));
        result = result && check("Test DFS", testDFS(graph, 7));
        result = result && check("Test DFS", !testDFS(graph, 5));
#endif
#if TEST_PATH_GEN
        result = result && check("Test path generation", testPathGen());
#endif
    }
    catch (const std::exception & ex)
    {
        std::cerr << ex.what() << std::endl;
        return false;
    }
    return result;
}

class V : public Graph<int>::Visitor
{
    public:
        virtual void visitNode (const int & i, std::set<int> adjacent) noexcept override
        {
            visited.push_back(i);
            // TODO something with adjacent??
        }

        std::vector<int> visited;
};

class Cell;

std::function<void(Graph<std::shared_ptr<const Cell>>::Path&)> pathReceiver;
//= std::bind(&RouteGenViaGraph::receivePath, std::reference_wrapper<RouteGenViaGraph>(*this), std::placeholders::_1);

bool test2 ()
{
    bool result = true;
    Graph<int> graph;
    result = result && check("Graph empty after initialized", graph.isEmpty());
    graph.addEdge(3, 6);
    graph.addDirectedEdge(6, 1);
    result = result && check("Graph not empty after edge added", !graph.isEmpty());
    graph.clear();
    result = result && check("Graph empty after clear()", graph.isEmpty());
    V visitor;
    graph.accept(visitor);
    result = result && check("No nodes visited for empty graph", visitor.visited.empty());
    graph.addEdge(4, 9);
    graph.addDirectedEdge(9, 3);
    graph.accept(visitor);
    result = result && check("All nodes visited", visitor.visited.size() == 3);
    result = result && check("Node 3 visited", std::find(visitor.visited.begin(), visitor.visited.end(), 3) != visitor.visited.end());
    result = result && check("Node 4 visited", std::find(visitor.visited.begin(), visitor.visited.end(), 4) != visitor.visited.end());
    result = result && check("Node 9 visited", std::find(visitor.visited.begin(), visitor.visited.end(), 9) != visitor.visited.end());
    // TODO
    //pathReceiver(std::bind(&RouteGenViaGraph::receivePath, std::reference_wrapper<RouteGenViaGraph>(*this), std::placeholders::_1))
    //graph.setEmitPathCallback(pathReceiver);
    return result;
}

int main ()
{
    std::cout << "Test graph" << std::endl;
    bool result = true;
    result = result && testGraph();
    result = result && test2();
    return result ? 0 : 1;
}
