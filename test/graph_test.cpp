#include "../app/Graph.h"

#include <iostream>
#include <memory>
#include <functional>
#include <algorithm>
#include <gtest/gtest.h>

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

bool receivePath (const std::vector<int> & path)
{
    std::cout << "Received path: ";
    for (auto it = std::begin(path); it != std::end(path); ++it)
    {
        std::cout << *it << ' ';
    }
    std::cout << std::endl;
    paths.push_back(path);
    return Graph<int>::CONTINUE_GENERATION;
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

TEST(graph_test, graph_path_gen)
{
    Graph<int> graph;

    graph.addEdge(3, 1);
    graph.addDirectedEdge(3, 6);
    graph.addEdge(2, 6);
    graph.addEdge(1, 2);
    graph.addEdge(4, 7); // <<<< Causes a divided graph, where not all nodes are reachable

    std::function<bool(std::vector<int> & path)> fn = receivePath;
    graph.setEmitPathCallback(&fn);
    std::cout << "All paths from 1 to 6:" << std::endl;
    paths.clear();
    graph.genAllPaths(1, 6);
    std::cout << std::endl;
    std::vector<int> expect1 = {1, 2, 6};
    std::vector<int> expect2 = {1, 3, 6};
    EXPECT_EQ(paths.size(), 2);
    // TODO check
    //(std::find(paths, expect1) != paths.end());
    //(std::find(paths, expect2) != paths.end());

    std::cout << "All paths from 6 to 3:" << std::endl;
    expect1 = {6, 2, 1, 3};
    paths.clear();
    graph.genAllPaths(6, 3);
    EXPECT_EQ(paths.size(), 1);
    EXPECT_TRUE(*paths.begin() == expect1);
    std::cout << std::endl;
}

TEST(graph_test, search)
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
    EXPECT_TRUE(testBFS(graph, 3));
    EXPECT_TRUE(testBFS(graph, 7));
    EXPECT_FALSE(testBFS(graph, 5));
#endif
#if TEST_DFS
    EXPECT_TRUE(testDFS(graph, 3));
    EXPECT_TRUE(testDFS(graph, 7));
    EXPECT_FALSE(testDFS(graph, 5));
#endif
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

TEST(graph_test, test2)
{
    Graph<int> graph;
    EXPECT_TRUE(graph.isEmpty());
    graph.addEdge(3, 6);
    graph.addDirectedEdge(6, 1);
    EXPECT_FALSE(graph.isEmpty());
    graph.clear();
    EXPECT_TRUE(graph.isEmpty());
    V visitor;
    graph.accept(visitor);
    EXPECT_TRUE(visitor.visited.empty());
    graph.addEdge(4, 9);
    graph.addDirectedEdge(9, 3);
    graph.accept(visitor);
    EXPECT_TRUE(visitor.visited.size() == 3);
    EXPECT_TRUE(std::find(visitor.visited.begin(), visitor.visited.end(), 3) != visitor.visited.end());
    EXPECT_TRUE(std::find(visitor.visited.begin(), visitor.visited.end(), 4) != visitor.visited.end());
    EXPECT_TRUE(std::find(visitor.visited.begin(), visitor.visited.end(), 9) != visitor.visited.end());
    // TODO more graph tests
    //pathReceiver(std::bind(&RouteGenViaGraph::receivePath, std::reference_wrapper<RouteGenViaGraph>(*this), std::placeholders::_1))
    //graph.setEmitPathCallback(pathReceiver);
}

static std::vector<std::vector<int>> validatorPaths;

#if 0
bool validator (const std::vector<int>& path)
{ return true; }
#else
bool validator (const std::vector<int>& path)
{
    std::cout << "Validate path: ";
    for (const int & val : path)
        std::cout << val << ' ';
    std::cout << std::endl;

    for (const int & val : path)
    {
        if (val > 9)
        {
            std::cout << "Invalid at " << val << std::endl;
            return false;
        }
        //std::cout << "Validated " << val << std::endl;
        validatorPaths.push_back(path); // This will do partial paths
        //std::cout << val << ' ';
    }
    std::cout << std::endl;
    return true;
}
#endif

TEST(graph_test, test_validation)
{
    std::cout << "Test validation" << std::endl;
    Graph<int> graph;
    std::function<bool(const std::vector<int>&)> fval = std::bind(&validator, std::placeholders::_1);
    graph.setValidatePathCallback(&fval);
    std::function<bool(std::vector<int> & path)> fn = receivePath;
    graph.setEmitPathCallback(&fn);

    graph.addEdge(3, 8);
    graph.addEdge(8, 1);
    graph.addEdge(8, 13);
    graph.addEdge(8, 24);
    graph.addEdge(24, 19);
    graph.addEdge(19, 1);
    graph.addEdge(19, 6);
    graph.addEdge(9, 13);
    graph.addEdge(5, 1);
    graph.addEdge(3, 9);
    graph.addEdge(9, 5);
    /*
             5 -- 1 --- 19
            /      \    | \
           |   3 -- 8   |  6
            \ /    /  \ |
             9 -- 13    24

       There are 2 valid routes from 1 to 3:
           1, 8, 3
           1, 5, 9, 3
     */

    GraphOutputter outputter(std::cout);
    graph.accept(outputter);

    paths.clear();
    std::cout << "Generate all paths" << std::endl;
    graph.genAllPaths(1, 3);
    std::cout << std::endl;

    std::cout << validatorPaths.size() << " paths processed by validator" << std::endl;
    for (auto path : validatorPaths)
    {
        for (int val : path)
            std::cout << val << ' ';
        std::cout << std::endl;
    }

    //result = result && check("Number of paths", validatorPaths.size() == 1);
    std::cout << paths.size() << " paths" << std::endl;
    for (auto path : paths)
    {
        for (int val : path)
            std::cout << val << ' ';
        std::cout << std::endl;
    }

    EXPECT_EQ(paths.size(), 2);

    std::cout << "-------------------------------" << std::endl;
    paths.clear();
    validatorPaths.clear();
    graph.genAllPaths(1, 5);
    std::cout << std::endl;
    /*
       There are 2 valid routes from 1 to 5:
           1, 5
           1, 8, 3, 9, 5
     */
    EXPECT_EQ(paths.size(), 2);

    /*std::cout << validatorPaths.size() << " paths processed by validator" << std::endl;
    for (auto path : validatorPaths)
    {
        for (int val : path)
            std::cout << val << ' ';
        std::cout << std::endl;
    }*/

    std::cout << "-------------------------------" << std::endl;
    paths.clear();
    validatorPaths.clear();
    graph.genAllPaths(1, 6);
    std::cout << std::endl;
    // There are no valid routes from 1 to 6
    EXPECT_EQ(paths.size(), 0);
}

// C++ program to print all paths
// from a source to destination.
#include <iostream>
#include <list>
using namespace std;

// A directed graph using
// adjacency list representation
class SomebodyElsesGraph {
    int V; // No. of vertices in graph
    list<int>* adj; // Pointer to an array containing
                    // adjacency lists

    // A recursive function used by printAllPaths()
    void printAllPathsUtil(int, int, bool[], int[], int&);

public:
    SomebodyElsesGraph(int V); // Constructor
    void addEdge(int u, int v);
    void printAllPaths(int s, int d);
};

SomebodyElsesGraph::SomebodyElsesGraph(int V)
{
    this->V = V;
    adj = new list<int>[V];
}

void SomebodyElsesGraph::addEdge(int u, int v)
{
    adj[u].push_back(v); // Add v to u’s list.
}

// Prints all paths from 's' to 'd'
void SomebodyElsesGraph::printAllPaths(int s, int d)
{
    // Mark all the vertices as not visited
    bool* visited = new bool[V];

    // Create an array to store paths
    int* path = new int[V];
    int path_index = 0; // Initialize path[] as empty

    // Initialize all vertices as not visited
    for (int i = 0; i < V; i++)
        visited[i] = false;

    // Call the recursive helper function to print all paths
    printAllPathsUtil(s, d, visited, path, path_index);
}

// A recursive function to print all paths from 'u' to 'd'.
// visited[] keeps track of vertices in current path.
// path[] stores actual vertices and path_index is current
// index in path[]
void SomebodyElsesGraph::printAllPathsUtil(int u, int d, bool visited[],
                            int path[], int& path_index)
{
    // Mark the current node and store it in path[]
    visited[u] = true;
    path[path_index] = u;
    path_index++;

    // If current vertex is same as destination, then print
    // current path[]
    if (u == d) {
        for (int i = 0; i < path_index; i++)
            cout << path[i] << " ";
        cout << endl;
    }
    else // If current vertex is not destination
    {
        // Recur for all the vertices adjacent to current
        // vertex
        list<int>::iterator i;
        for (i = adj[u].begin(); i != adj[u].end(); ++i)
            if (!visited[*i])
                printAllPathsUtil(*i, d, visited, path,
                                path_index);
    }

    // Remove current vertex from path[] and mark it as
    // unvisited
    path_index--;
    visited[u] = false;
}

// Driver program
int somebodyElses()
{
    std::cout << "With an implementation by somebody else" << std::endl;
    // Create a graph given in the above diagram
#if 0//THEIR_EXAMPLE
    SomebodyElsesGraph g(4);
    g.addEdge(0, 1);
    g.addEdge(0, 2);
    g.addEdge(0, 3);
    g.addEdge(2, 0);
    g.addEdge(2, 1);
    g.addEdge(1, 3);

    int s = 2, d = 3;
    cout << "Following are all different paths from " << s
        << " to " << d << endl;
    g.printAllPaths(s, d);
#else
    // Note their example above is a directed graph.
    // So to duplicate my test graph, create edges both directions here.
    SomebodyElsesGraph g(100); // Value must be enough to store all paths
    g.addEdge(3, 8);    g.addEdge(8, 3);
    g.addEdge(8, 1);    g.addEdge(1, 8);
    g.addEdge(8, 13);   g.addEdge(13, 8);
    g.addEdge(8, 24);   g.addEdge(24, 8);
    g.addEdge(24, 19);  g.addEdge(19, 24);
    g.addEdge(19, 1);   g.addEdge(1, 19);
    g.addEdge(19, 6);   g.addEdge(6, 19);
    g.addEdge(9, 13);   g.addEdge(13, 9);
    g.addEdge(5, 1);    g.addEdge(1, 5);
    g.addEdge(3, 9);    g.addEdge(9, 3);
    g.addEdge(9, 5);    g.addEdge(5, 9);
    int s = 1, d = 5;
    cout << "Following are all different paths from " << s
        << " to " << d << endl;
    g.printAllPaths(s, d);
#endif

    return 0;
}
