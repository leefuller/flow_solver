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
 * Add pipe identifier to set of identifiers, if the cell at coordinate has endpoint PipeEnd::PIPE_START. Otherwise, do nothing.
 * @param cell      Cell to assess
 */
void Solver::addPipeIdToIdSetIfCellIsStart (ConstCellPtr cell) noexcept
{
    if (cell->getEndpoint() == PipeEnd::PIPE_START)
        m_pipeIds.insert(cell->getPipeId());
}

static unsigned countRoutes = 0;
static unsigned countRoutesDiscarded = 0;

/**
 * Calls the designated route generator which emits routes as they are generated.
 * @param idPipe    Pipe identifier for routes
 */
void Solver::generateRoutes (PipeId idPipe)
{
        logger << "Solver: Generate routes for " << idPipe << std::endl;
        countRoutes = 0;
        countRoutesDiscarded = 0;

        m_routeGen.setReceiver(this);

        Coordinate start = m_puzzle->findPipeEnd(idPipe, PipeEnd::PIPE_START);
        Coordinate end = m_puzzle->findPipeEnd(idPipe, PipeEnd::PIPE_END);
        // Follow fixtures to get to the point where the path is unknown
        Route route;
        route.push_back(start);
        ConstCellPtr pCell = m_puzzle->getConstCellAtCoordinate(start);
        Direction forward = Direction::NONE;
        for (bool fixed = true; fixed && pCell->getEndpoint() != PipeEnd::PIPE_END;)
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
            return;
        }

        logger << "Generating routes for " << idPipe << std::endl;
        // NOTE: The route generator does not explicitly trace existing fixtures from start point.
        // The traverseToCreateGraph checks for NO_CONNECTOR to skip making a useless graph edge,
        // as long as the preliminary stuff has done a good job.
        // So that means the graph creation implicitly tracks the already assigned fixtures.
        m_routeGen.generateRoutes(idPipe, m_puzzle);
        m_routeGen.removeReceiver();
        logger << countRoutes << " routes for " << idPipe << std::endl;
        if (countRoutesDiscarded)
            logger << countRoutesDiscarded << " routes discarded" << std::endl;
}

/**
 * Add given route to list of routes for pipe
 * @param idPipe    Pipe identifier
 * @param route     Route to add
 */
void Solver::addRoute (PipeId idPipe, const Route & route) noexcept
{
    m_routeList.push_back(std::pair<PipeId, Route>(idPipe, route));
}

/**
 * Callback from route generator for discovered route.
 * @param idPipe    Identifier of pipe
 * @param route     Route generated
 * @return Graph::STOP_GENERATION to indicate route generation can stop
 */
bool Solver::processRoute (PipeId idPipe, Route & route)
{
    ++countRoutes;
    try
    {
#if ANNOUNCE_SOLVER_DETAIL
        if (interestingPipe(idPipe))
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

            if (m_pipeIds.size() > 1)
            {
                // Create solver with puzzle copy having pipe inserted.
                std::set<PipeId> remainingPipes(m_pipeIds);
                remainingPipes.erase(idPipe);
                Solver solver(m_puzzle, remainingPipes);
                if (route.size() > 1)
                {
                    Coordinate prevCoord = route[0];
                    for (unsigned i = 1; i < route.size(); ++i)
                    {
                        Direction dFrom = areAdjacent(prevCoord, route[i]);
                        CellPtr pCell1 = solver.m_puzzle->getCellAtCoordinate(prevCoord);
                        if (pCell1->getConnection(dFrom) != CellConnection::FIXTURE_CONNECTION)
                        {
                            CellPtr pCell2 = solver.m_puzzle->getCellAtCoordinate(route[i]);
#if ANNOUNCE_SOLVER_DETAIL
                            logger << "Connect cells " << prevCoord << " to " << route[i] << std::endl;
#endif
                            try
                            {
                                solver.connectAndRevise(pCell1, pCell2, CellConnection::FIXTURE_CONNECTION);
                            }
                            catch (PuzzleException & ex)
                            {
                                ex.addContext(SOURCE_REF, "connect " + coordinateToString(pCell1->getCoordinate()) + 
                                " to " + coordinateToString(pCell2->getCoordinate()) + " in processing route " + routeToString(route));
                                throw;
                            }
                        }
                        prevCoord = route[i];
                    }
                }
                // Call next solver
                logger << "Solve for next pipe. Remaining = " << remainingPipes.size() << std::endl;
                if (solver.solve())
                {
                    setSolved();
                }
            }
            else
            {
                setSolved(checkSolution(m_routeList.begin(), m_routeList.end()));
            }
            if (isSolved())
            {
                logger << "Solved" << std::endl;
                return Graph<ConstCellPtr>::STOP_GENERATION;
            }
        }
    }
    catch (const PuzzleException & ex)
    {
        if (interestingPipe(idPipe))
        {
            logger << "Caught exception in processing route " << route << std::endl;
            logger << "Exception processing route: " << ex << std::endl;
        }
        throw;
    }
    return Graph<ConstCellPtr>::CONTINUE_GENERATION;
}

/**
 * Check if Routes in list given by iterators are a solution.
 * Requires one route per pipe id.
 */
bool Solver::checkSolution (std::vector<std::pair<PipeId, Route>>::iterator start, std::vector<std::pair<PipeId, Route>>::iterator end) noexcept
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
        if (!isCorner(m_puzzle, coord2))
            continue; // try next direction

        ConstCellPtr pCellNext = m_puzzle->getConstCellAtCoordinate(coord2);
        // For a corner to be part of a channel, a cell next to it must be a channel, or an opposing corner.

        // The corner must have a wall in the same direction. and a wall 90 degrees to direction.
        ConstCellPtr pCellNextAgain = nullptr;
        Coordinate coord3 = coord;
        switch (direction)
        {
            case Direction::NORTH:
                //[[fallthrough]]
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
                //[[fallthrough]]
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
 * @param pCell     Cell bring revised
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
    try
    {
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
                    try 
                    {
                        if (m_puzzle->getPlumber()->removeConnector(pCell, d))
                        {
                            changed = true;
                            result.insert(pCell);
                        }
                    }
                    catch (PlumberException & ex)
                    {
                        ex.addContext(SOURCE_REF, "remove connection " + asString(d) + " from " + coordinateToString(pCell->getCoordinate()));
                        throw;
                    }
                    // remove connector from adjacent cell, if any
                    CellPtr pAdj = m_puzzle->getCellAdjacent(pCell->getCoordinate(), d);
                    if (pAdj != nullptr)
                    {
                        try
                        {
                            if (m_puzzle->getPlumber()->removeConnector(pAdj, opposite(d)))
                            {
                                changed = true;
                                result.insert(pAdj);
                            }
                        }
                        catch (PlumberException & ex)
                        {
                            ex.addContext(SOURCE_REF, "remove connection " + asString(opposite(d)) + " from " + coordinateToString(pAdj->getCoordinate()));
                            throw;
                        }
                    }
                }
                continue;
            }
            // else pCell can accept connection
            if (pCell->isFixture() && pCell->getBorder(d) != CellBorder::WALL)
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
                                    if (m_puzzle->traceRoute(pCell->getPipeId(), PipeEnd::PIPE_START, route))
                                    {
    #if ANNOUNCE_SOLVER_DETAIL
                                        logger << "Found route " << std::endl;
    #endif
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
    }
    catch (PuzzleException & ex)
    {
        ex.addContext(SOURCE_REF, "revise cell at " + coordinateToString(pCell->getCoordinate()));
        throw;
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
    if (m_puzzle->traceRoute(pCellFrom->getPipeId(), PipeEnd::PIPE_START, route))
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
            try
            {
                std::set<CellPtr> r = reviseCell(p);
                for (CellPtr pr : r)
                    reviseMore.insert(pr);
            }
            catch (PuzzleException & ex)
            {
                ex.addContext(SOURCE_REF, "revise after connect pipe " + 
                    std::to_string(pCellFrom->getPipeId()) + " at " + 
                    coordinateToString(pCellFrom->getCoordinate()) + " to " + coordinateToString(pCellAdjacent->getCoordinate()));
                throw;
            }
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
    {
        route.push_back(p->getCoordinate());
        //if (p->getCoordinate() == createCoordinate(7,1))
          //  logger << "*******************************" << std::endl;
    }
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
        if (interestingPipe(idPipe))
        {
            logger << "Caught exception in processing route " << route << std::endl;
            logger << "Exception processing route: " << ex << std::endl;
        }
        throw;
    }
    return true;
}

/**
 * Connect given pipe to adjacent cell if there is only 1 possible pipe for the adjacent cell,
 * and it has the same id as the cell p.
 * @param pCheck         Cell for which possible connection is checked
 * @param changed
 */
void Solver::connectIfOnlyOnePossibility (CellPtr pCheck, bool & changed)
{
    if (pCheck == nullptr)
        return;
    if (pCheck->isFixture())
    {
        std::set<Direction> sameIdsAdjacent; // directions to adjacent cells matching cell pipe id
        for (Direction dirToAdjacent : allTraversalDirections)
        {
            if (pCheck->getBorder(dirToAdjacent) == CellBorder::WALL)
                continue;
            CellPtr pAdj = m_puzzle->getCellAdjacent(pCheck->getCoordinate(), dirToAdjacent);
            if (pAdj == nullptr)
                continue;
            if (pAdj->getPipeId() == UNREACHABLE_CELL_DEF_CH)
                continue;

            // Adjacent cell is directly reachable, disregarding it's current status
            const std::set<PipeId> & idsPossible = pAdj->getPossiblePipes(); // set of pipe ids possible at adjacent cell
#if ANNOUNCE_SOLVER_DETAIL
            logger << pCheck->getCoordinate() << " " << asString(dirToAdjacent) << " has " << idsPossible.size() << " possibilities";
            for (PipeId id : idsPossible)
                logger << "'" << id << "' ";
            logger << std::endl;
#endif
            if (idsPossible.size() == 1)
            {
                PipeId id = *std::begin(idsPossible);
                if (id != pCheck->getPipeId())
                    continue;
                if (pCheck->getConnection(dirToAdjacent) == CellConnection::FIXTURE_CONNECTION)
                    continue;
#if ANNOUNCE_SOLVER_DETAIL
                logger << "Connect with only 1 possibility at " << pAdj->getCoordinate() << std::endl;
#endif
                try
                {
                    connectAndRevise(pCheck, pAdj, CellConnection::FIXTURE_CONNECTION);
                    changed = true; // TODO not sure if this should return immediately
                }
                catch (PuzzleException & ex)
                {
                    ex.addContext(SOURCE_REF, "connect for only 1 possibility");
                    throw;
                }
            }
            else if (idsPossible.find(pCheck->getPipeId()) != idsPossible.end())
            {
                sameIdsAdjacent.insert(dirToAdjacent);
            }
        }
        if (sameIdsAdjacent.size() == 1)
        {
            Direction d = *std::begin(sameIdsAdjacent);
            if (pCheck->getConnection(d) == CellConnection::FIXTURE_CONNECTION || pCheck->getConnection(d) == CellConnection::NO_CONNECTOR)
                return;
            CellPtr pAdj = m_puzzle->getCellAdjacent(pCheck->getCoordinate(), d);
#if ANNOUNCE_SOLVER_DETAIL
            logger << "Connect with only 1 possible direction at " << pAdj->getCoordinate() << std::endl;
#endif
            try
            {
                connectAndRevise(pCheck, pAdj, CellConnection::FIXTURE_CONNECTION);
                changed = true;
            }
            catch (PuzzleException & ex)
            {
                ex.addContext(SOURCE_REF, "connect for only 1 possible direction");
                throw;
            }
        }
    }
}

/**
 * Check if there is an obstruction after 1 cell in direction from given cell,
 * and if the obstruction is a pipe, that it is not the same pipe as in the given cell.
 * @param pCell     Cell to check
 * @param dirCheck  Direction to check from cell
 * @return true if path is obstructed after 1 empty cell in the given direction
 */
bool Solver::checkObstructionAfter1 (ConstCellPtr pCell, Direction dirCheck)
{
    if (pCell == nullptr)
        return false;
#if ANNOUNCE_SOLVER_DETAIL
    logger << "check obstruction " << asString(dirCheck) << " after 1" << std::endl;
#endif
    if (!pCell->isBorderOpen(dirCheck)) // obstructed by a wall
        return false;

    Coordinate c = pCell->getCoordinate();
    if (bool obstruct = true; m_puzzle->gapToObstruction(c, dirCheck) == 1)
    {
        CellPtr pAdj = m_puzzle->getCellAdjacent(c, dirCheck);
        if (pAdj != nullptr)
        {
            if (pAdj->isBorderOpen(dirCheck)) // Obstruction is a pipe
            {
                CellPtr pObstruct = nullptr;
                if (Coordinate coordObstruction = c; coordinateChange(coordObstruction, dirCheck, 2))
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
         .|     Cell in the corner cannot be 'X', unless a start or end point for a different pipe occupies a cell adjacent to the corner,
       X        and the corner contains a start or endpoint for pipe X.

      If there is an obstruction 1 step north or east of 'X', then the cell between
      (marked 'o') cannot be 'X'.
      If the obstruction is a wall (cases 2a and 3a), the statement is always correct.
      FIXME If the obstruction is a pipe, the statement is only true if the obstructing pipe cannot accept another connection.
      If the obstructing pipe is able to accept another connection (cases 2b and 3b), then the cell marked 'o' could possibly be 'X'.

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
        if (pCorner->isEndpoint())
        {
            // TODO can something be derived/updated here?
            return;
        }
#if ANNOUNCE_SOLVER_DETAIL
        logger << asString(dCorner) << " corner from " << c << " removes possibility of " << idPipe << " at " << pCorner->getCoordinate() << std::endl;
#endif
        pCorner->removePossibility(idPipe);

        // Check corner for obstruction on 1 axis
        Direction dAxis = rotateLeft(dCorner);
        CellPtr pAxis = m_puzzle->getCellAdjacent(c, dAxis);
        if (checkObstructionAfter1(pCell, dAxis))
        {
#if ANNOUNCE_SOLVER_DETAIL
            logger << asString(dCorner) << " from " << c << " corner with " << asString(dAxis) <<
                    " obstruction removes possibility of " << idPipe << " at " << pAxis->getCoordinate() << std::endl;
#endif
            pAxis->removePossibility(idPipe);
        }

        // Check corner for obstruction on other axis
        dAxis = rotateRight(dCorner);
        pAxis = m_puzzle->getCellAdjacent(c, dAxis);
        if (checkObstructionAfter1(pCell, dAxis))
        {
#if ANNOUNCE_SOLVER_DETAIL
            logger << asString(dCorner) << " from " << c << " corner with " << asString(dAxis)
                    << " obstruction removes possibility of " << idPipe << " at " << pAxis->getCoordinate() << std::endl;
#endif
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
        try
        {
            connectAndRevise(pCell, m_puzzle->getCellAtCoordinate(dest), CellConnection::FIXTURE_CONNECTION);
            changed = true;
        }
        catch (PuzzleException & ex)
        {
            ex.addContext(SOURCE_REF, "connect " + asString(d) + " towards obstruction");
            throw;
        }
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
        try
        {
            connectAndRevise(pCellFrom, pCellAdjacent, CellConnection::FIXTURE_CONNECTION);
        }
        catch (PuzzleException & ex)
        {
            ex.addContext(SOURCE_REF, "connect " + asString(oneWay) + " only from " + coordinateToString(pCellFrom->getCoordinate()));
            throw;
        }

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

bool Solver::solve() noexcept
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

        // Remove completed pipes from set
        m_prelimRoutes.clear();
        std::set<PipeId> remainingPipes;
        for (PipeId id : m_pipeIds)
        {
            Route route;
            if (m_puzzle->traceRoute(id, PipeEnd::PIPE_START, route))
                m_prelimRoutes[id] = route;
            else
                remainingPipes.insert(id);
            route.clear();
        }
        logger << "Pipes remaining: " << remainingPipes.size() << std::endl;
        if (remainingPipes.empty())
            return Puzzle::checkIfSolution(m_puzzle, m_prelimRoutes);

        m_pipeIds = remainingPipes;

        // Initialize possibilities for every cell that is not a fixture
        std::set<PipeId> & allPipesSet = m_pipeIds;

        // Set pipe possibilities
        std::function<void(CellPtr)> lam = [this, allPipesSet](CellPtr cell){
            if (cell == nullptr)
                return;
            if (!m_puzzle->isCellReachable(cell->getCoordinate()))
                return;
            if (cell->isFixture())
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
        Cell::setOutputConnectorRep(true);
        m_puzzle->streamPuzzleMatrix(std::cout);

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

        std::cout << "After preliminary phase:" << std::endl;
        Cell::setOutputConnectorRep(true);
        m_puzzle->streamPuzzleMatrix(std::cout);

#if ANNOUNCE_SOLVER_DETAIL
        logger << "Possible cell contents:" << std::endl;
        listCellPossibilities(m_puzzle);
#endif

        //---------------------------------------------------------

        logger << "Check solution after preliminary phase" << std::endl;
        if (Puzzle::checkIfSolution(m_puzzle, m_prelimRoutes))
            return true;

        //---------------------------------------------------------

        // For each pipe, find routes between endpoints.
        logger << "Generating routes..." << std::endl;

        // Choose 1 of the pipe ids
        PipeId idPipe = *m_pipeIds.begin();
        generateRoutes(idPipe);
        //generateRoutes(m_prelimRoutes);
        return isSolved();
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

/**
 * Create a new Solver with a copy of a puzzle
 * @param p         Puzzle to be copied
 * @param ids       Set of pipe identifiers
 */
Solver::Solver (const PuzzlePtr p, const std::set<PipeId> & ids)
  : m_puzzleDef(p->definition()), m_puzzle(new Puzzle(*p)), m_pipeIds(ids), m_routeGen(m_puzzle)
{}
