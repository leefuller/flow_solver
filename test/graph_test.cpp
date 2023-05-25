//#include "GraphTest.h"
#include "../app/Graph.h"
#include "test_helper.h"

#include <iostream>
#include <memory>
#include <functional>

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

void printPath (std::vector<int> & path)
{
    std::cout << "Received path: ";
    for (auto it = std::begin(path); it != std::end(path); ++it)
    {
        std::cout << *it << ' ';
    }
    std::cout << std::endl;
}

#define TEST_BFS    1
#define TEST_DFS    1
#define TEST_PATH_GEN   1

static bool testBFS (Graph<int> & graph, int lookup)
{
    std::cout << "Run breadth first search for " << lookup << std::endl;
    try
    {
        int i = graph.breadthFirstSearch([lookup](int found) { return lookup == found; });
        if (i != lookup)
            return false;
        std::cout << "found" << std::endl;
        return true;
    }
    catch (Graph<int>::GraphException & ex)
    {
        std::cerr << ex.what() << std::endl;
    }
    return false;
}

static bool testDFS (Graph<int> & graph, int lookup)
{
    std::cout << "Run depth first search for " << lookup << std::endl;
    try
    {
        int i = graph.depthFirstSearch([lookup](int found) { return lookup == found; });
        if (i != lookup)
            return false;
        std::cout << "found" << std::endl;
        return true;
    }
    catch (Graph<int>::GraphException & ex)
    {
        std::cerr << ex.what() << std::endl;
    }
    return false;
}

static void testPathGen (Graph<int> & graph)
{
    std::function<void(std::vector<int> & path)> fn = printPath;
    graph.setEmitPathCallback(&fn);
    graph.genAllPaths(1, 6);
    std::cout << std::endl;

    graph.genAllPaths(6, 3);
    std::cout << std::endl;
    // TODO check
}

bool testGraph ()
{
    std::cout << "Graph test" << std::endl;
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
        testBFS(graph, 3);
        testBFS(graph, 7);
#endif
#if TEST_DFS
        testDFS(graph, 3);
        testDFS(graph, 7);
#endif
#if TEST_PATH_GEN
        testPathGen(graph);
#endif
    }
    catch (const std::exception & ex)
    {
        std::cerr << ex.what() << std::endl;
        return false;
    }
    return true;
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
    result = result && check(graph.isEmpty());
    graph.addEdge(3, 6);
    graph.addDirectedEdge(6, 1);
    result = result && check(!graph.isEmpty());
    graph.clear();
    result = result && check(graph.isEmpty());
    V visitor;
    graph.accept(visitor);
    result = result && check(visitor.visited.empty());
    graph.addEdge(4, 9);
    graph.addDirectedEdge(9, 3);
    graph.accept(visitor);
    result = result && check(visitor.visited.size() == 3);
    result = result && check(std::find(visitor.visited.begin(), visitor.visited.end(), 3) != visitor.visited.end());
    result = result && check(std::find(visitor.visited.begin(), visitor.visited.end(), 4) != visitor.visited.end());
    result = result && check(std::find(visitor.visited.begin(), visitor.visited.end(), 9) != visitor.visited.end());
    // TODO
    //pathReceiver(std::bind(&RouteGenViaGraph::receivePath, std::reference_wrapper<RouteGenViaGraph>(*this), std::placeholders::_1))
    //graph.setEmitPathCallback(pathReceiver);

    // TODO
    //graph.genAllPaths (NodeT source, NodeT dest)
    //graph.depthFirstSearch (std::function<bool(NodeT&)> checkFn)
    //graph.breadthFirstSearch (std::function<bool(NodeT&)> checkFn)
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
