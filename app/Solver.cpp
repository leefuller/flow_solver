#include <map>
#include <set>
#include <vector>
#include <list>
#include <string>
#include <tuple>
#include <stdexcept>
#include <algorithm> // std::find
#include <functional>

#include "Matrix.h"
#include "RouteGenViaGraph.h"
#include "Solver.h"
#include "../include/PuzzleException.h"
#include "formations.h"
#include "../include/Logger.h"

static Logger & logger = Logger::getDefaultLogger();

/**
 * Add pipe identifier to set of identifiers, if the cell at coordinate has endpoint PipeEnd::PIPE_END_1. Otherwise, do nothing.
 * @param cell      Cell to assess
 */
void Solver::addPipeIdToIdSetIfCellIsStart (CellPtr cell) noexcept
{
    if (cell->getEndpoint() == PipeEnd::PIPE_END_1)
        m_pipeIds.insert(cell->getPipeId());
}

static unsigned countRoutes = 0;
static unsigned countRoutesDiscarded = 0;

/**
 * Calls the designated route generator which emits routes as they are generated.
 * @param existing  Route data already existing
 */
void Solver::generateRoutes (const std::map<PipeId, Route> & existing)
{
    logger << "Solver: Generate routes" << std::endl;
    for (PipeId idPipe : m_pipeIds)
    {
        countRoutes = 0;
        countRoutesDiscarded = 0;

        m_routeGen.addReceiver(this);

        Coordinate start = m_puzzle->findPipeEnd(idPipe, PipeEnd::PIPE_END_1);
        Coordinate end = m_puzzle->findPipeEnd(idPipe, PipeEnd::PIPE_END_2);
        // Follow fixtures to get to the point where the path is unknown
        Route route;
        route.push_back(start);
        ConstCellPtr pCell = m_puzzle->getConstCellAtCoordinate(start);
        Direction forward = Direction::NONE;
        for (bool fixed = true; fixed && pCell->getEndpoint() != PipeEnd::PIPE_END_2;)
        {
            fixed = false;
            for (Direction direction : allTraversalDirections)
            {
                if (direction != Direction::NONE && direction == opposite(forward))
                    continue;
                if (pCell->getConnection(direction) == CellConnection::FIXTURE_CONNECTION)
                {
                    forward = direction;
                    pCell = m_puzzle->getConstCellAdjacent(pCell->getCoordinate(), direction);
                    route.push_back(pCell->getCoordinate());
                    fixed = true;
                    break;
                }
            }
        }
        if (route.back() == end) // Route complete
        {
            processRoute(idPipe, route);
            continue;
        }

        // TODO multi-threaded (thread per pipe). Each thread would need a copy of the puzzle.
        logger << "Generating routes for " << idPipe << std::endl;
        m_routeGen.generateRoutes(idPipe, m_puzzle);//, existing[idPipe]);
        m_routeGen.removeReceiver(this);
        logger << countRoutes << " routes for " << idPipe << std::endl;
        logger << countRoutesDiscarded << " routes discarded" << std::endl;
    }
}

/**
 * Add given route to list of routes for pipe
 * @param idPipe    Pipe identifier
 * @param route     Route to add
 */
void Solver::addRoute (PipeId idPipe, const Route & route)
{
    if (m_routesDict.find(idPipe) == m_routesDict.end())
    {
        // Create empty route list for pipe
        logger << "Create list of routes for " << idPipe << std::endl;
        m_routesDict[idPipe] = std::vector<Route>();
    }
    m_routesDict[idPipe].push_back(route);
}

/**
 * Use to inject a route into a puzzle.
 * Automatically removes it when this is destroyed.
 */
class TryRoute
{
    public:
        TryRoute (PuzzlePtr puzzle, PipeId idPipe, const Route & route)
            : m_puzzle(puzzle), m_route(route)
        {
            m_puzzle->insertRoute(idPipe, route);
        }

        ~TryRoute ()
        {
            m_puzzle->removeRoute();
        }

    private:
        PuzzlePtr m_puzzle;
        Route m_route;
};

const PipeId interestingPipe = 'P';

/**
 * Callback from route generator for discovered route.
 * @param idPipe    Identifier of pipe
 * @param route     Route generated
 */
void Solver::processRoute (PipeId idPipe, Route & route)
{
    ++countRoutes;
    try
    {
        TryRoute t(m_puzzle, idPipe, route);

        if (detectBadFormation(m_puzzle, route, idPipe))
            ++countRoutesDiscarded;
        else
            addRoute(idPipe, route);
    }
    catch (const PuzzleException & ex)
    {
        if (idPipe == interestingPipe)
        {
            logger << "Caught exception in processing route " << route << std::endl;
            logger << "Exception processing route: " << ex << std::endl;
        }
        throw;
    }
}

/**
 * Check if Routes in list given by iterators are a solution.
 * Requires one route per pipe id.
 */
bool Solver::checkSolution (std::vector<std::pair<PipeId, Route>>::iterator start, std::vector<std::pair<PipeId, Route>>::iterator end)
{
    std::map<PipeId, Route> m;
    for (; start != end; start++)
    {
        if (m.find(start->first) != m.end())
            return false; // PipeId already seen. Solution only has one route per pipe.
        m[start->first] = start->second;
    }
    return Puzzle::checkIfSolution(m_puzzle, m);
}

// Recursive template function to process combinations
// (From n items, process combinations of r items)
template <class RanIt, class Func>
bool recursive_combination (PuzzlePtr puzzle, RanIt nbegin, RanIt nend, int n_column,
    RanIt rbegin, RanIt rend, int r_column, int loop, 
    Func func)
{
    int r_size = rend - rbegin; // r
    int localloop = loop; //
    int local_n_column = n_column; //

    // A different combination is out
    if (r_column > (r_size - 1))
    {
        // func to return true to end recursion
        return (*func)(rbegin, rend);
    }
    //===========================

    for (int i = 0; i <= loop; ++i)
    {
        RanIt it1 = rbegin;
        for (int cnt = 0; cnt < r_column; ++cnt)
            ++it1;
        RanIt it2 = nbegin;
        for (int cnt2 = 0; cnt2 < n_column+i; ++cnt2)
            ++it2;

        *it1 = *it2;
        ++local_n_column;

        if (recursive_combination(puzzle, nbegin, nend, local_n_column, rbegin, rend, r_column + 1, localloop, func))
            return true;
        --localloop;
    }
    return false;
}

/**
 * Check whether a coordinate is adjacent to the start of a channel, but not in the channel.
 * A channel opening can be at a corner.
 * @return true if the coordinate is at a channel opening
 */
bool Solver::isAdjacentToChannelOpening (Coordinate coord) const noexcept
{
    ConstCellPtr pCell = m_puzzle->getConstCellAtCoordinate(coord);

    for (Direction direction : allTraversalDirections)
    {
        if (!pCell->isBorderOpen(direction))
            continue; // try next direction

        // Set coord2 as adjacent in direction
        Coordinate coord2 = coord;
        if (!coordinateChange(coord2, direction))
            return false;
        // Check for channel in direction
        if ((isVertical(direction) && isVerticalChannel(coord2)) ||
                (!isVertical(direction) && isHorizontalChannel(coord2)))
            return true;

        // Check for entry to a channel via a corner.
        if (!Helper::isCorner(m_puzzle, coord2))
            continue; // try next direction

        ConstCellPtr pCellNext = m_puzzle->getConstCellAtCoordinate(coord2);
        // For a corner to be part of a channel, a cell next to it must be a channel, or an opposing corner.

        // The corner must have a wall in the same direction. and a wall 90 degrees to direction.
        ConstCellPtr pCellNextAgain = nullptr;
        Coordinate coord3 = coord;
        switch (direction)
        {
            case Direction::NORTH:
                [[fallthrough]]
            case Direction::SOUTH:
                if (coordinateChange(coord3, pCellNext->getBorder(Direction::WEST) == CellBorder::OPEN ? Direction::WEST : Direction::EAST))
                    return false;
                // We already checked that it is not a vertical channel, so get direction of corner
                pCellNextAgain = m_puzzle->getConstCellAtCoordinate(coord3);
                // If that cell is a channel in the new direction,
                // or an opposite corner, then we have a channel confirmed
                if (pCellNextAgain->isHorizontalChannel() || pCellNextAgain->getBorders() == getOppositeBorder(pCellNext->getBorders()))
                    return true;
                // else not a valid corner for channel
                break;

            case Direction::WEST:
                [[fallthrough]]
            case Direction::EAST:
                // We already checked that it is not a horizontal channel, so get direction of corner
                if (coordinateChange(coord3,pCellNext->getBorder(Direction::NORTH) == CellBorder::OPEN ? Direction::NORTH : Direction::SOUTH))
                    return false;
                pCellNextAgain = m_puzzle->getConstCellAtCoordinate(coord3);
                // If that cell is a channel in the new direction,
                // or an opposite corner, then we have a channel confirmed
                if (pCellNextAgain->isHorizontalChannel() || pCellNextAgain->getBorders() == getOppositeBorder(pCellNext->getBorders()))
                    return true;
                // else not a valid corner for channel
                break;
            case Direction::NONE:
                break;
        }
    }
    return false;
}

static void describeConnection (ConstCellPtr cellFrom, ConstCellPtr cellAdjacent)
{
    std::cout << "Connect from ";
    cellFrom->describe(std::cout);
    std::cout << " to ";
    cellAdjacent->describe(std::cout);
}

/**
 * For each cell adjacent to the given cell, check connectors between them.
 * - If any adjacent pair of cells with common id have an open fixture facing each other,
 *   then it can be changed to a fixture, if allowed.
 * - A non-fixed connection next to a fixed endpoint can be removed.
 * @return cells where any change is made
 */
std::set<CellPtr> Solver::reviseCell (CellPtr pCell) noexcept(false)
{
    logger << "Revise " << pCell->getCoordinate() << std::endl;
    std::set<CellPtr> result;
    bool changed = false;
    for (Direction d : allTraversalDirections)
    {
        if (pCell->getConnection(d) == CellConnection::NO_CONNECTOR)
            continue; // Disconnected direction is never restored

        if (!pCell->canAcceptConnection(d))
        {
            // ensure no connector for direction
            if (pCell->getConnection(d) != CellConnection::FIXTURE_CONNECTION &&
                pCell->getConnection(d) != CellConnection::NO_CONNECTOR)
            {
                changed = m_puzzle->getPlumber()->removeConnector(pCell, d);
                if (changed)
                    result.insert(pCell);
            }
            continue;
        }
        // else can accept connection
        if (pCell->isFixture())
        {
            CellPtr pAdj = m_puzzle->getCellAdjacent(pCell->getCoordinate(), d);
            if (pAdj != nullptr)
            {
                if (pAdj->isFixture())
                {
                    if (pAdj->getPipeId() != pCell->getPipeId())
                    {
                        // Ensure no connector between adjacent fixtures for different pipes
                        if (pCell->getConnection(d) != CellConnection::NO_CONNECTOR)
                        {
                            changed = m_puzzle->getPlumber()->removeConnector(pCell, d);
                            if (changed)
                            {
                                result.insert(pCell);
                                result.insert(pAdj); // adjacent could change accordingly
                            }
                            continue;
                        }
                    }
                    else // next to same pipe
                    {
                        if (pAdj->getConnection(opposite(d)) != CellConnection::NO_CONNECTOR)
                        {
                            if (pAdj->canAcceptConnection(opposite(d)))
                            {
                                // Adjacent cell can connect
                                m_puzzle->getPlumber()->connect(pCell->getCoordinate(), pAdj->getCoordinate(),
                                        pCell->getPipeId(), CellConnection::FIXTURE_CONNECTION);
                                changed = true;
                                // need to revise both connected
                                result.insert(pCell);
                                result.insert(pAdj); // adjacent could change accordingly
                                continue;
                            }
                        }
                    }
                }
            }
        }
        // else pCell is not a fixture
    }
    return result;
}

bool Solver::solve()
{
    m_puzzle->streamPuzzleMatrix(std::cout);
    unsigned phase = 0;
    try
    {
        // Visit each cell to find all pipe start points, and build the set of pipe identifiers.
        std::function<void(CellPtr)> f =
                std::bind(&Solver::addPipeIdToIdSetIfCellIsStart, std::reference_wrapper<Solver>(*this),
            std::placeholders::_1); // Placeholder for CellPtr
        m_puzzle->forEveryCell(&f);
        logger << "Pipes expected: " << m_pipeIds.size() << std::endl;

        // Initialize possibilities for every cell that is not an endpoint
        std::set<PipeId> & allPipesSet = m_pipeIds;
        // Set pipe possibilities
        std::function<void(CellPtr)> lam = [allPipesSet](CellPtr cell){
            if (!cell->isEndpoint())
            { cell->setPossiblePipes(allPipesSet); }
        };
        m_puzzle->forEveryCell(&lam);

        std::map<PipeId, Route> prelimRoutes;

        ++phase;
        //---------------------------------------------------------
        // Phase 1: Run the "only one way" rule until the puzzle stops changing
        logger << "Solving: Preliminary phase " << phase << std::endl;

        for (bool changed = true; changed;)
        {
            changed = false;
            for (int r = 0; r < m_puzzle->getNumRows(); ++r)
            {
                for (int c = 0; c < m_puzzle->getNumCols(); ++c)
                {
                    Direction oneWay = theOnlyWay(m_puzzle, {r,c});
                    if (oneWay != Direction::NONE)
                    {
                        changed = true;
                        CellPtr cellFrom = m_puzzle->getCellAtCoordinate({r,c});
                        CellPtr cellAdjacent = m_puzzle->getCellAdjacent({r,c}, oneWay);
                        m_puzzle->getPlumber()->connect(cellFrom->getCoordinate(), cellAdjacent->getCoordinate(),
                                cellFrom->getPipeId(), CellConnection::FIXTURE_CONNECTION);

                        // Check other cells to be revised after plumber action
                        std::set<CellPtr> toRevise = { cellFrom, cellAdjacent };
                        std::set<CellPtr> reviseMore;
                        do
                        {
                            for (CellPtr p : toRevise)
                            {
                                std::set<CellPtr> r = reviseCell(p);
                                for (CellPtr pr : r)
                                    reviseMore.insert(pr);
                            };
                            toRevise = reviseMore;
                            reviseMore.clear();
                        } while (!toRevise.empty());

                        // Check if a route has been formed
                        Route route = {};
                        if (m_puzzle->traceRoute(cellFrom->getPipeId(), route))
                        {
                            logger << "Found route using one way rule" << std::endl;
                            prelimRoutes.insert_or_assign(cellFrom->getPipeId(), route);
                        }

                        m_puzzle->streamPuzzleMatrix(std::cout);
                    }
                }
            }
        }

        std::cout << "After one way rule applied:" << std::endl;
        Cell::setOutputConnectorRep(true);
        m_puzzle->streamPuzzleMatrix(std::cout);

        // TODO
        // Run "one from corner" algorithm to reduce possibilities there
        //forEveryCell(checkOneFromCorner())

        // TODO
        // Run bad formation detectors

        //---------------------------------------------------------

        logger << "Check solution after preliminary phase" << std::endl;
        if (Puzzle::checkIfSolution(m_puzzle, prelimRoutes))
            return true;

        //---------------------------------------------------------

        // For each pipe, find routes between endpoints.
        logger << "Generating routes..." << std::endl;
        generateRoutes(prelimRoutes);

        if (m_routesDict.size() != m_pipeIds.size())
        {
            logger << "Solution impossible. No valid route found for a pipe." << std::endl;
            logger << "Expect " << m_pipeIds.size() << " pipes. Have " << m_routesDict.size() << std::endl;
            return false;
        }

        logger << "Generated routes for " << m_routesDict.size() << " pipes..." << std::endl;
        std::set<PipeId> solvedRoutes;
        // Generate a list of routes, as a list of pairs (id, route)
        for (std::pair<const PipeId, std::vector<Route>> & pipeRouteList : m_routesDict) // std::map<PipeId, std::vector<Route>> m_routesDict;
        {
            PipeId idPipe = pipeRouteList.first;
            logger << "Pipe " << pipeRouteList.first << " has " << pipeRouteList.second.size() << " routes" << std::endl;
            for (Route & route : pipeRouteList.second)
                logger << route << std::endl;

            if (pipeRouteList.second.size() == 1)
                solvedRoutes.insert(idPipe);
        }

        // Generate route list excluding routes from other pipes if they contain any coordinate in a solved route.
        for (std::pair<const PipeId, std::vector<Route>> & pipeRouteList : m_routesDict)
        {
            PipeId idPipe = pipeRouteList.first;
            if (solvedRoutes.find(idPipe) != solvedRoutes.end())
            {
                m_routeList.push_back(std::pair<PipeId, Route>(idPipe, pipeRouteList.second[0]));
                continue;
            }
            for (Route & route : pipeRouteList.second) // for each route for idPipe
            {
                bool exclude = false;
                for (PipeId idSolved : solvedRoutes)
                {
                    Route & routeSolved = m_routesDict[idSolved][0];
                    for (Coordinate & c : route)
                    {
                        if (coordinateInRoute(c, routeSolved))
                        {
                            // Route can be excluded
                            exclude = true;
                            break;
                        }
                    }
                    if (exclude)
                        break;
                }
                if (!exclude)
                    m_routeList.push_back(std::pair<PipeId, Route>(idPipe, route));
            }
        }

        logger << m_routeList.size() << " routes generated." << std::endl;

        //---------------------------------------------------------

        logger << "Searching for solution..." << std::endl;
        // Find combination of pipes (1 per id) where endpoints are reached without intersection of pipes (ie. without any shared coordinate)
        unsigned r = m_pipeIds.size();
        std::vector<std::pair<PipeId, Route>> comb(r);
        unsigned n = m_routeList.size();
        std::function<bool(std::vector<std::pair<PipeId, Route>>::iterator, std::vector<std::pair<PipeId, Route>>::iterator)> fCheckSolution
            = std::bind(&Solver::checkSolution, std::reference_wrapper<Solver>(*this),
                std::placeholders::_1, std::placeholders::_2);
        return recursive_combination(m_puzzle, m_routeList.begin(), m_routeList.end(), 0, comb.begin(), comb.end(), 0, n - r, &fCheckSolution);
    }
    catch (const std::exception & ex)
    {
        logger << "Exception: " << ex.what() << std::endl;
    }
    return false;
}

Solver::Solver (const char * puzzleDef)
  : m_puzzleDef(puzzleDef), m_puzzle(m_puzzleDef.generatePuzzle()), m_routeGen(m_puzzle)
{}

extern const char * getPuzzleDef ();

int main (int argc, const char * argv[])
{
    try
    {
        Solver solver(getPuzzleDef());
        solver.solve();
    }
    catch (const PuzzleException & ex)
    {
    	std::cerr << ex << std::endl;
    }
    catch (const std::exception & ex)
    {
        std::cerr << ex.what() << std::endl;
    }
    catch (const char * s)
    {
        std::cerr << s << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unexpected object thrown" << std::endl;
    }

    logger << "Exit." << std::endl;
    return 0;
}
