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
void Solver::addPipeIdToIdSetIfCellIsStart (ConstCellPtr cell) noexcept
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
                // Follow existing fixtures from start of route
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
        // NOTE: The route generator does not explicitly trace existing fixtures from start point.
        // The traverseToCreateGraph checks for NO_CONNECTOR to skip making a useless graph edge,
        // as long as the preliminary stuff has done a good job.
        // So that means the graph creation implicitly tracks the already assigned fixtures.
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

const PipeId interestingPipe = 'N';

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
#if ANNOUNCE_SOLVER_DETAIL
        if (idPipe == interestingPipe)
            std::cout << idPipe << ": " << route << std::endl;
#endif
        TryRoute t(m_puzzle, idPipe, route);

        if (detectBadFormation(m_puzzle, route, idPipe))
        {
            ++countRoutesDiscarded;
#if ANNOUNCE_SOLVER_DETAIL
            logger << "Discard route " << route << std::endl;
#endif
        }
        else
        {
#if ANNOUNCE_ROUTE_FOUND
            std::cout << idPipe << " route found: " << route << std::endl;
            Cell::setOutputConnectorRep(false);
            m_puzzle->streamPuzzleMatrix(std::cout);
#endif
            addRoute(idPipe, route);
        }
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

/*bool updateConnectorsBetweenCells (CellPtr pCell, Direction d)
{
    if (pCell == nullptr)
        return false;
    bool changed = false;
    CellPtr pAdj = m_puzzle->getCellAdjacent(pCell->getCoordinate(), d);
    if (pAdj == nullptr)
    {
        // Should be no connector from pCell for direction
        changed = m_puzzle->getPlumber()->removeConnector(pCell, d);
    }
    else // There is an adjacent cell in direction
    {
        if (!pCell->canAcceptConnection(d))
        {
            changed = m_puzzle->getPlumber()->removeConnector(pCell, d);
            changed = changed || m_puzzle->getPlumber()->removeConnector(pAdj, opposite(d));
        }
    }
    return changed;
}*/

/**
 * For each cell adjacent to the given cell, check connectors between them.
 * - If any adjacent pair of cells with common id have an open fixture facing each other,
 *   then it can be changed to a fixture, if allowed.
 * - A non-fixed connection next to a fixed endpoint can be removed.
 * @return cells where any change is made
 */
std::set<CellPtr> Solver::reviseCell (CellPtr pCell) noexcept(false)
{
#if ANNOUNCE_SOLVER_DETAIL
    logger << "Revise " << pCell->getCoordinate() << std::endl;
    pCell->describe(logger.stream()); logger << std::endl;
#endif
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
                if (m_puzzle->getPlumber()->removeConnector(pCell, d))
                {
                    changed = true;
                    result.insert(pCell);
                }
                // remove connector from adjacent cell, if any
                CellPtr pAdj = m_puzzle->getCellAdjacent(pCell->getCoordinate(), d);
                if (pAdj != nullptr)
                {
                    // remove connector from adjacent cell, if any
                    if (m_puzzle->getPlumber()->removeConnector(pAdj, opposite(d)))
                    {
                        changed = true;
                        result.insert(pAdj);
                    }
                }
            }
            continue;
        }
        // else pCell can accept connection
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
                            if (m_puzzle->getPlumber()->removeConnector(pCell, d))
                            {
                                changed = true;
                                result.insert(pCell);
                                //result.insert(pAdj); // adjacent could change accordingly
                            }
                            // remove connector from adjacent cell, if any
                            CellPtr pAdj = m_puzzle->getCellAdjacent(pCell->getCoordinate(), d);
                            if (pAdj != nullptr)
                            {
                                // remove connector from adjacent cell, if any
                                if (m_puzzle->getPlumber()->removeConnector(pAdj, opposite(d)))
                                {
                                    changed = true;
                                    result.insert(pAdj);
                                }
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

                                pCell->setPossiblePipes(pAdj->getPipeId());
                                // if pipe is complete, update possibilities for all other puzzle cells
                                Route route;
                                if (m_puzzle->traceRoute(pCell->getPipeId(), route))
                                {
                                    logger << "Found route " << std::endl;
                                    m_prelimRoutes.insert_or_assign(pCell->getPipeId(), route);
                                    updateRemovePossibleForAllOther(pCell->getPipeId());
                                }

                                changed = true;
                                // need to revise both connected
                                result.insert(pCell);
                                result.insert(pAdj); // adjacent could change accordingly
                                continue;
                            }
                        }
                    }
                }
                // else pAdj is not a fixture
            }
        }
        // else pCell is not a fixture
    }
    return result;
}

/**
 * For any cell not containing a given pipe,
 * remove it as a possibility.
 */
void Solver::updateRemovePossibleForAllOther (PipeId idPipe) noexcept
{
    // for other cells, remove this pipe as a possibility
    std::function<void(CellPtr)> lam = [idPipe](CellPtr p){
        if (p == nullptr)
            return;
        if (p->getPipeId() != idPipe)
            p->removePossibility(idPipe);
    };
    m_puzzle->forEveryCellMutable(&lam);
}

/**
 * Make connection between 2 cells, and revise cell state.
 */
void Solver::connectAndRevise (CellPtr pCellFrom, CellPtr pCellAdjacent, CellConnection con)
{
    m_puzzle->getPlumber()->connect(pCellFrom->getCoordinate(), pCellAdjacent->getCoordinate(),
            pCellFrom->getPipeId(), con);

    if (con == CellConnection::FIXTURE_CONNECTION)
        pCellAdjacent->setPossiblePipes(pCellFrom->getPipeId());

    // Check if a route has been formed
    Route route;
    if (m_puzzle->traceRoute(pCellFrom->getPipeId(), route))
    {
        logger << "Found route " << std::endl;
        m_prelimRoutes.insert_or_assign(pCellFrom->getPipeId(), route);
        updateRemovePossibleForAllOther(pCellFrom->getPipeId());
    }

    // Check other cells to be revised after plumber action
    std::set<CellPtr> toRevise = { pCellFrom, pCellAdjacent };
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
}

/**
 * Called during route generation to check validity of path being generated.
 */
bool Solver::validatePath (const std::vector<ConstCellPtr> & path)
{
    if (path.empty())
        return true;
    PipeId idPipe = path[0]->getPipeId();
    Route route;
    for (auto p : path)
        route.push_back(p->getCoordinate());
    try
    {
        TryRoute t(m_puzzle, idPipe, route);

        // TODO would be good if the route generator could use the algorithms used in preliminary solution process,
        // so that less routes are generated.

        if (detectBadFormation(m_puzzle, route, idPipe))
        // Don't use adjacency law, because it could invalidate a cell in assessing a bad route,
        // where that cell can be in the solution
        //if (detectDeadEndFormation(m_puzzle, route, idPipe))
        {
            // No need to do anything to the puzzle here. The route will be removed when leaving this function.
            // The route generator deals with the removal of the cell (which was last added) from the route.
            return false;
        }
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
    return true;
}

void Solver::connectIfOnlyOnePossibility (CellPtr p, bool & changed)
{
    if (p == nullptr)
        return;
    if (p->isFixture())
    {
        std::set<Direction> sameIdsAdjacent;
        for (Direction d : allTraversalDirections)
        {
            CellPtr pAdj = m_puzzle->getCellAdjacent(p->getCoordinate(), d);
            if (pAdj == nullptr)
                continue;
            if (pAdj->getPipeId() == UNREACHABLE_CELL_DEF_CH)
                continue;
            const std::set<PipeId> & ids = pAdj->getPossiblePipes();
            logger << p->getCoordinate() << " " << asString(d) << " has " << ids.size() << " possibilities";
            for (PipeId id : ids)
                logger << "'" << id << "' ";
            logger << std::endl;
            if (ids.size() == 1)
            {
                PipeId id = *std::begin(ids);
                if (id != p->getPipeId())
                    continue;
                if (p->getConnection(d) == CellConnection::FIXTURE_CONNECTION)
                    continue;
                logger << "Connect with only 1 possibility at " << pAdj->getCoordinate() << std::endl;
                connectAndRevise(p, pAdj, CellConnection::FIXTURE_CONNECTION);
                changed = true;
            }
            else if (ids.find(p->getPipeId()) != ids.end())
            {
                sameIdsAdjacent.insert(d);
            }
        }
        if (sameIdsAdjacent.size() == 1)
        {
            Direction d = *std::begin(sameIdsAdjacent);
            if (p->getConnection(d) == CellConnection::FIXTURE_CONNECTION || p->getConnection(d) == CellConnection::NO_CONNECTOR)
                return;
            CellPtr pAdj = m_puzzle->getCellAdjacent(p->getCoordinate(), d);
            logger << "Connect with only 1 possibile direction at " << pAdj->getCoordinate() << std::endl;
            connectAndRevise(p, pAdj, CellConnection::FIXTURE_CONNECTION);
            changed = true;
        }
    }
}

/**
 * Check if there is an obstruction after 1 cell in direction from given cell,
 * and if the obstruction is a pipe, that it is not the same pipe as in the given cell.
 */
bool Solver::checkObstructionAfter1 (ConstCellPtr pCell, Direction d)
{
    if (pCell == nullptr)
        return false;
    logger << "check obstruction " << asString(d) << " after 1" << std::endl;
    if (!pCell->isBorderOpen(d)) // obstructed by a wall
        return false;

    Coordinate c = pCell->getCoordinate();
    if (bool obstruct = true; m_puzzle->gapToObstruction(c, d) == 1)
    {
        CellPtr pAdj = m_puzzle->getCellAdjacent(c, d);
        if (pAdj != nullptr)
        {
            if (pAdj->isBorderOpen(d)) // Obstruction is a pipe
            {
                CellPtr pObstruct = nullptr;
                if (Coordinate coordObstruction = c; coordinateChange(coordObstruction, d, 2))
                {
                    pObstruct = m_puzzle->getCellAtCoordinate(coordObstruction);
                    if (pObstruct->getPipeId() == pCell->getPipeId())
                        obstruct = false; // Matching pipe is not a terminating obstruction
                }
            }
            // else obstruction is a border
        }
        return obstruct;
    }
    return false;
}

/**
 * Check corner formation for given direction of corner
 * @param pCell
 * @param dCorner   Direction from cell to corner
 * @return
 */
void Solver::checkCornerFormation (CellPtr pCell, Direction dCorner)
{
    if (pCell == nullptr)
        return;
    if (pCell->isEmpty())
        return;

    /* Illustration of meaning, using NORTH_EAST corner example:

       (1)
        ==
         .|     Cell in the corner cannot be 'X'.
       X

      If there is an obstruction 1 step north or east of 'X', then the cell between
      (marked 'o') cannot be 'X'. Obstruction can be border or pipe.

        (2a)   (2b)            (3a)     (3b)
        ===    A B              ==       A
        o .|   o . C             .|      o B
        X      X               X o|    X   C

     */
    PipeId idPipe = pCell->getPipeId();
    Coordinate c = pCell->getCoordinate();

    if (checkOneStepToCorner(m_puzzle, c, dCorner) == dCorner)
    {
        // Case (1) Given cell is diagonal from corner
        CellPtr pCorner = m_puzzle->getCellAdjacent(c, dCorner);
        logger << asString(dCorner) << " corner from " << c << " removes possibility of " << idPipe << " at " << pCorner->getCoordinate() << std::endl;
        pCorner->removePossibility(idPipe);

        // Check corner for obstruction on 1 axis
        Direction dAxis = rotateLeft(dCorner);
        CellPtr pAxis = m_puzzle->getCellAdjacent(c, dAxis);
        if (checkObstructionAfter1(pCell, dAxis))
        {
            logger << asString(dCorner) << " from " << c << " corner with " << asString(dAxis) <<
                    " obstruction removes possibility of " << idPipe << " at " << pAxis->getCoordinate() << std::endl;
            pAxis->removePossibility(idPipe);
        }

        // Check corner for obstruction on other axis
        dAxis = rotateRight(dCorner);
        pAxis = m_puzzle->getCellAdjacent(c, dAxis);
        if (checkObstructionAfter1(pCell, dAxis))
        {
            logger << asString(dCorner) << " from " << c << " corner with " << asString(dAxis)
                    << " obstruction removes possibility of " << idPipe << " at " << pAxis->getCoordinate() << std::endl;
            pAxis->removePossibility(idPipe);
        }
    }
}

void Solver::checkCornerFormations (CellPtr pCell, bool & changed)
{
    if (pCell == nullptr)
        return;
    checkCornerFormation(pCell, Direction::NORTH_EAST);
    checkCornerFormation(pCell, Direction::NORTH_WEST);
    checkCornerFormation(pCell, Direction::SOUTH_EAST);
    checkCornerFormation(pCell, Direction::SOUTH_WEST);
}

void Solver::checkFillToObstruction (CellPtr pCell, bool & changed)
{
    if (pCell == nullptr)
        return;
    if (pCell->getPipeId() == NO_PIPE_ID)
        return;

    Direction d = checkFillToCorner(m_puzzle, pCell);
    if (d != Direction::NONE)
    {
#if ANNOUNCE_ONE_WAY_DETECT
        logger << "Connect from " << pCell->getCoordinate() << " towards obstruction " << asString(d) << std::endl;
#endif
        // Connect cell towards obstruction
        // If the obstruction is actually another step away, it will be dealt with in another iteration
        Coordinate dest = pCell->getCoordinate();
        coordinateChange(dest, d);
        connectAndRevise(pCell, m_puzzle->getCellAtCoordinate(dest), CellConnection::FIXTURE_CONNECTION);
        changed = true;
    }
}

void Solver::checkOneWay (CellPtr pCellFrom, bool & changed)
{
    if (pCellFrom == nullptr)
        return;
    Direction oneWay = theOnlyWay(m_puzzle, pCellFrom->getCoordinate());
    if (oneWay != Direction::NONE)
    {
        changed = true;
        CellPtr pCellAdjacent = m_puzzle->getCellAdjacent(pCellFrom->getCoordinate(), oneWay);
        connectAndRevise(pCellFrom, pCellAdjacent, CellConnection::FIXTURE_CONNECTION);

        //m_puzzle->streamPuzzleMatrix(std::cout);
    }
}

void listCellPossibilities (const PuzzlePtr p)
{
    std::function<void(ConstCellPtr)> lam = [p](ConstCellPtr pCell) {
    //auto lam = [](ConstCellPtr pCell) {
        if (!p->isCellReachable(pCell->getCoordinate()))
            return;
        if (pCell->isFixture())
            return;
        logger << pCell->getCoordinate() << ": ";
        for (auto pp : pCell->getPossiblePipes())
            logger << pp << ' ';
        logger << std::endl;
    };
    p->forEveryCell(&lam);
}

bool Solver::solve()
{
    m_puzzle->streamPuzzleMatrix(std::cout);
    try
    {
        std::function<bool(const std::vector<ConstCellPtr>&)> fval =
                std::bind(&Solver::validatePath, std::reference_wrapper<Solver>(*this), std::placeholders::_1);
        m_routeGen.setPathValidator(&fval);
        // Visit each cell to find all pipe start points, and build the set of pipe identifiers.
        std::function<void(ConstCellPtr)> f =
                std::bind(&Solver::addPipeIdToIdSetIfCellIsStart, std::reference_wrapper<Solver>(*this),
            std::placeholders::_1); // Placeholder for CellPtr
        m_puzzle->forEveryCell(&f);
        logger << "Pipes expected: " << m_pipeIds.size() << std::endl;

        // Initialize possibilities for every cell that is not an endpoint
        std::set<PipeId> & allPipesSet = m_pipeIds;
        // Set pipe possibilities
        std::function<void(CellPtr)> lam = [this, allPipesSet](CellPtr cell){
            if (cell == nullptr)
                return;
            if (!m_puzzle->isCellReachable(cell->getCoordinate()))
                return;
            if (cell->isEndpoint())
            { cell->setPossiblePipes(cell->getPipeId()); }
            else
            { cell->setPossiblePipes(allPipesSet); }
        };
        m_puzzle->forEveryCellMutable(&lam);

        /* If endpoints are diagonally adjacent, without intervening walls,
           there are only 2 possible paths for each,
           regardless of empty cells on any other side.
                X .     is always either   X X  or X .
                . X                        . X     X X
            TODO handle diagonally adjacent cells
        */

        m_prelimRoutes.clear();
        bool changed = true;

        //---------------------------------------------------------
        unsigned phase = 1;
        // Phase 1: Run the "only one way" rule until the puzzle stops changing
        logger << "Solving: Preliminary phase" << std::endl;
        while (changed)
        {
            while (changed) // inner loop for strict one way algorithm must run before other
            {
                changed = false;
                std::function<void(CellPtr)> f1 = std::bind(&Solver::checkOneWay, std::reference_wrapper<Solver>(*this),
                        std::placeholders::_1, changed); // Placeholder for CellPtr
                m_puzzle->forEveryCellMutable(&f1);
            }

#if 1
            ++phase;
            // Run one way fill to obstruction algorithm.
            // This algorithm cannot be run until the prior one way detections are run,
            // otherwise it could fill a space that is one way for a different pipe.
            // Also, if this changes the puzzle, then need to run the prior algorithms again.
#if ANNOUNCE_ONE_WAY_DETECT
            logger << "Try one way to obstruction algorithm" << std::endl;
#endif
            changed = false;
#if 0
            std::function<void(CellPtr)> f1 = std::bind(&Solver::checkFillToObstruction, std::reference_wrapper<Solver>(*this),
                    std::placeholders::_1, changed); // Placeholder for CellPtr
            m_puzzle->forEveryCellMutable(&f1);
#else
            // Cannot run the Puzzle::forEveryCell, because we want to bail out on the first change
            for (int r = 0; r < m_puzzle->getNumRows(); ++r)
            {
                for (int c = 0; c < m_puzzle->getNumCols(); ++c)
                {
                    CellPtr pCell = m_puzzle->getCellAtCoordinate({r,c});
                    if (pCell->getPipeId() == NO_PIPE_ID)
                        continue;

                    checkFillToObstruction(pCell, changed);
                    if (changed)
                        break; // must return to run one way algorithm again
                }
                if (changed)
                    break; // must return to run one way algorithm again
            }
            if (changed)
                continue;
#endif
#endif
            ++phase;
            // Check corner formations
            std::function<void(CellPtr)> f2 = std::bind(&Solver::checkCornerFormations, std::reference_wrapper<Solver>(*this),
                    std::placeholders::_1, changed); // Placeholder for CellPtr
            m_puzzle->forEveryCellMutable(&f2);

            ++phase;
            // Check to see if any cell now has only one possibility
            std::function<void(CellPtr)> f3 = std::bind(&Solver::connectIfOnlyOnePossibility, std::reference_wrapper<Solver>(*this),
                    std::placeholders::_1, changed); // Placeholder for CellPtr
            m_puzzle->forEveryCellMutable(&f3);

            if (changed)
                phase = 1;
        }

        std::cout << "After one way rule applied:" << std::endl;
        Cell::setOutputConnectorRep(true);
        m_puzzle->streamPuzzleMatrix(std::cout);

        logger << "Possible cell contents:" << std::endl;
        listCellPossibilities(m_puzzle);

        //---------------------------------------------------------

        logger << "Check solution after preliminary phase" << std::endl;
        if (Puzzle::checkIfSolution(m_puzzle, m_prelimRoutes))
            return true;

        //---------------------------------------------------------

        // For each pipe, find routes between endpoints.
        logger << "Generating routes..." << std::endl;
        generateRoutes(m_prelimRoutes);

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
            {
                logger << route << std::endl;
                // Insert route into puzzle to show in output
                m_puzzle->insertRoute(idPipe, route);
                Cell::setOutputConnectorRep(false);
                m_puzzle->streamPuzzleMatrix(std::cout);
                m_puzzle->removeRoute();
            }

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
        bool solved = solver.solve();
        if (!solved)
            logger << "No solution found" << std::endl;
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
