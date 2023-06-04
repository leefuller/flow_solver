#include "../app/formations.h"
#include "../include/Puzzle.h"
#include "../include/Cell.h"
#include "test_helper.h"

bool testBad2x2 ()
{
    bool result = true;
    const char * samplePuzzleDef1 = \
    {" = = = = = = = ,"  \
     "|. . . . . . A|,"  \
     "|             |,"  \
     "|A . . . B . B|,"  \
     " = = = = = = = "   \
    };
    PuzzleDefinition def1(samplePuzzleDef1);
    std::shared_ptr<Puzzle> p = def1.generatePuzzle();
    std::shared_ptr<Cell> pCell = p->getCellAtCoordinate({1,5});
    pCell->setPipeId('B');

    // inject route causing adjacency issue
    pCell = p->getCellAtCoordinate({0,0});
    pCell->setPipeId('A');
    std::shared_ptr<Cell> pCellAdjacent = p->getCellAtCoordinate({1,0});
    //pCell->connectFixture(Direction::UP, pCellAdjacent);

    pCell = p->getCellAtCoordinate({0,1});
    pCell->setPipeId('A');
    //std::shared_ptr<Cell> pCellAdjacent = p->getCellAtCoordinate({0,2});
    //pCell->connectFixture(Direction::UP, pCellAdjacent);

    pCell = p->getCellAtCoordinate({0,2});
    pCell->setPipeId('A');

    pCell = p->getCellAtCoordinate({0,3});
    pCell->setPipeId('A');

    pCell = p->getCellAtCoordinate({1,2});
    pCell->setPipeId('A');

    pCell = p->getCellAtCoordinate({1,3});
    pCell->setPipeId('A');
    /* Formation has a 2x2 adjacency issue:
     = = = = = = =
    |A A A A . . A|
    |             |
    |A . A A B B B|
     = = = = = = =
     */

    //Route routeA = {{1,0},{0,0},{0,1},{0,2},{1,2},{1,3},{0,3}}; // Partial route that breaks adjacency rule
    Route routeA = {{1,0},{0,0},{0,1},{0,2},{1,2},{1,3},{0,3},{0,4},{0,5},{0,6}}; // Complete route that breaks adjacency rule
    // TODO or should it be a route next to that formation?
    result = result && check("Route A breaks adjacency law", adjacencyRuleBroken(p, routeA));
    Route routeB = {{1,4},{1,5},{1,6}};
    result = result && check("Route B does not break adjacency law", !adjacencyRuleBroken(p, routeB));
    return result;
}

bool testDivided2x2 ()
{
    bool result = true;
    const char * samplePuzzleDef1 = \
    {" = = = = = = = ,"  \
     "|. . .|. . . A|,"  \
     "|             |,"  \
     "|A . . . B . B|,"  \
     " = = = = = = = "   \
    };
    PuzzleDefinition def1(samplePuzzleDef1);
    std::shared_ptr<Puzzle> p = def1.generatePuzzle();
    std::shared_ptr<Cell> pCell = p->getCellAtCoordinate({1,5});
    pCell->setPipeId('B');

    // inject route causing adjacency issue
    pCell = p->getCellAtCoordinate({0,0});
    pCell->setPipeId('A');
    std::shared_ptr<Cell> pCellAdjacent = p->getCellAtCoordinate({1,0});
    //pCell->connectFixture(Direction::UP, pCellAdjacent);

    pCell = p->getCellAtCoordinate({0,1});
    pCell->setPipeId('A');
    //std::shared_ptr<Cell> pCellAdjacent = p->getCellAtCoordinate({0,2});
    //pCell->connectFixture(Direction::UP, pCellAdjacent);

    pCell = p->getCellAtCoordinate({0,2});
    pCell->setPipeId('A');

    pCell = p->getCellAtCoordinate({0,3});
    pCell->setPipeId('A');

    pCell = p->getCellAtCoordinate({1,2});
    pCell->setPipeId('A');

    pCell = p->getCellAtCoordinate({1,3});
    pCell->setPipeId('A');
    /* Formation has a no 2x2 adjacency issue because of dividing wall:
     = = = = = = =
    |A A A|A . . A|
    |             |
    |A . A A B B B|
     = = = = = = =
     */

    Route routeA = {{1,0},{0,0},{0,1},{0,2},{1,2},{1,3},{0,3},{0,4},{0,5},{0,6}};
    result = result && check("Route A does not break adjacency law", !adjacencyRuleBroken(p, routeA));
    return result;
}

int main ()
{
    bool result = true;
    result = result && testBad2x2();
    result = result && testDivided2x2();
    return result ? 0 : 1;
}
