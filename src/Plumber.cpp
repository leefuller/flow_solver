#include "Plumber.h"
#include "Cell.h"
#include "Puzzle.h"
#include "Logger.h"

static Logger & logger = Logger::getDefaultLogger();

PlumberException::PlumberException (const char * fmt,...) noexcept
{
    va_list args;
    va_start(args, fmt);
    setMsg(buildString(fmt, args));
    va_end(args);
}

static void plumberCheck (bool expect, bool condition, const char * msg) noexcept(false)
{
    if (condition != expect)
        throw PlumberException(msg);
}

/** Helper function to throw PlumberException on condition false.
 * @param msg   Message in exception
 */
static void plumberCheckTrue (bool condition, const char * msg) noexcept(false)
{ plumberCheck(true, condition, msg); }

/** Helper function to throw PlumberException on condition true.
 * @param msg   Message in exception
 */
static void plumberCheckFalse (bool condition, const char * msg) noexcept(false)
{ plumberCheck(false, condition, msg); }

/**
 * Remove connector from cell, if possible.
 * @return true if the cell connection status was changed
 * @throw PlumberException if removal of connector would leave the cell invalid (so impossible to solve the puzzle)
 */
bool Plumber::removeConnector (CellPtr pCell, Direction d)
{
    if (pCell->getConnection(d) == CellConnection::NO_CONNECTOR)
        return false; // Nothing to do
    unsigned countConnectors = std::count_if(std::begin(pCell->m_connection), std::end(pCell->m_connection),
            [](CellConnection c){ return c != CellConnection::NO_CONNECTOR; });
    if (pCell->isEndpoint() && countConnectors == 1)
        throw PlumberException("cannot remove last connection from endpoint");
    if (!pCell->isEndpoint() && countConnectors == 2)
        throw PlumberException("cannot have less than 2 connections for cell");

    pCell->setConnection(d, CellConnection::NO_CONNECTOR);
    return true;
}

/**
 * Connect cells at given coordinates. Coordinates should be adjacent.
 * Checks that it is possible to do so, before anything is done.
 * Connection can occur if:
 * - Both cells have an open connector towards each other.
 * - One coordinate must contain a matching pipe already.
 *   (Plumber cannot connect a pipe in the middle of nowhere)
 *
 * When the connection is deemed ok to proceed, the result is:
 * - If either open connector is on a fixture, then both become fixtures.
 * - For a cell becoming a new fixture, but not yet having 2 fixtures connected,
 *   the open connectors are changed to open fixture connectors.
 *
 * @param c1    Coordinate for one end of the connection
 * @param c2    Coordinate for other end of the connection
 * @param idPipe    Pipe identifier.
 * @param con       Connection type. Only accepts FIXTURE_CONNECTION or TEMPORARY_CONNECTION.
 * @throw PuzzleException if any condition prevents connection.
 */
void Plumber::connect (Coordinate c1, Coordinate c2, PipeId idPipe, CellConnection con) const noexcept(false)
{
    logger << "Plumb " << con << " from " << c1 << " to " << c2 << " for pipe " << idPipe << std::endl;
    if (con != CellConnection::FIXTURE_CONNECTION && con != CellConnection::TEMPORARY_CONNECTION)
        throw PlumberException("attempt invalid connection %d", con);

    Direction dFrom = areAdjacent(c1, c2); // Direction from c1 to c2, if adjacent
    plumberCheckFalse(dFrom == Direction::NONE, "cannot connect cells not adjacent");

    CellPtr pCell1 = m_puzzle->getCellAtCoordinate(c1);
    plumberCheckFalse(pCell1 == nullptr, "attempt to connect cell not existing");
    CellPtr pCell2 = m_puzzle->getCellAtCoordinate(c2);
    plumberCheckFalse(pCell2 == nullptr, "attempt to connect cell not existing");

    if (pCell1->getPipeId() == NO_PIPE_ID && pCell2->getPipeId() == NO_PIPE_ID)
        throw PlumberException("nothing to connect");

    plumberCheckFalse(pCell1->getPipeId() != idPipe && pCell1->getPipeId() != NO_PIPE_ID, "attempt to connect incompatible pipes");
    plumberCheckFalse(pCell2->getPipeId() != idPipe && pCell2->getPipeId() != NO_PIPE_ID, "attempt to connect incompatible pipes");

    // Check adjacent connectors
    plumberCheckFalse(pCell1->getConnection(dFrom) == CellConnection::NO_CONNECTOR, "attempt to connect where no connector exists");
    plumberCheckFalse(pCell1->getConnection(dFrom) == CellConnection::FIXTURE_CONNECTION, "attempt to connect where fixed connection already exists");
    plumberCheckFalse(pCell2->getConnection(opposite(dFrom)) == CellConnection::NO_CONNECTOR, "attempt to connect where no opposite connector exists");
    plumberCheckFalse(pCell2->getConnection(opposite(dFrom)) == CellConnection::FIXTURE_CONNECTION, "attempt to connect where fixed connection already exists");

    // Check attempt to set more than 1 fixture to endpoint, or more than 2 otherwise
    unsigned count = pCell1->countFixtureConnections();
    if (pCell1->isEndpoint() && count == 1)
        throw PlumberException("attempt to connect extra fixed connection to end point");
    else if (count > 2) // cannot connect if cell already has 2 fixed connections
        throw PlumberException("attempt to connect extra fixed connection");
    count = pCell2->countFixtureConnections();
    if (pCell2->isEndpoint() && count == 1)
        throw PlumberException("attempt to connect extra fixed connection to end point");
    else if (count > 2) // cannot connect if cell already has 2 fixed connections
        throw PlumberException("attempt to connect extra fixed connection");

    // Ok to connect
    pCell2->setPipeId(pCell1->getPipeId());
    if (con == CellConnection::FIXTURE_CONNECTION)
    {
        // Both connectors become fixed connection
        pCell1->setConnection(dFrom, CellConnection::FIXTURE_CONNECTION);
        pCell2->setConnection(opposite(dFrom), CellConnection::FIXTURE_CONNECTION);

        // Open connectors on fixtures become open fixture connectors
        for (Direction d : allTraversalDirections)
        {
            if (pCell1->getConnection(d) == CellConnection::OPEN_CONNECTOR)
                pCell1->setConnection(d, CellConnection::OPEN_FIXTURE);
            if (pCell2->getConnection(d) == CellConnection::OPEN_CONNECTOR)
                pCell2->setConnection(d, CellConnection::OPEN_FIXTURE);
        }

        // The plumber is not responsible to connect adjacent other open fixtures automatically.
        // ie. If after connection just done, another could be done due to the changes,
        // then it is not done here. It should be part of solution generator logic.
        // The plumber only does what it is told directly.
    }
    else if (con == CellConnection::TEMPORARY_CONNECTION)
    {
        // TODO? not required for current solution process
    }
    // else CellConnection::OPEN_FIXTURE is a result of other work. It is not a logical request.

    else // CellConnection::OPEN_CONNECTOR, CellConnection::NO_CONNECTOR
    {
        // TODO? not required for current solution process. Otherwise:
        // TODO probably use disconnect for CellConnection::NO_CONNECTOR
        // TODO or implement here to catch any out of range condition (which should only occur due to a bug)

    }
}

/*
void Plumber::disconnect (Coordinate c1, Coordinate c2) const noexcept(false)
{
    Direction d = areAdjacent(c1, c2); // Direction from c1 to c2, if adjacent
    if (d == Direction::NONE)
        throw PlumberException("attempt to disconnect cells not adjacent");

    CellPtr pCell1 = m_puzzle->getCellAtCoordinate(c1);
    if (pCell1 == nullptr)
        throw PlumberException("attempt to disconnect cell not existing");
    CellPtr pCell2 = m_puzzle->getCellAtCoordinate(c1);
    if (pCell2 == nullptr)
        throw PlumberException("attempt to disconnect cell not existing");

    if (pCell1->getPipeId() == NO_PIPE_ID && pCell2->getPipeId() == NO_PIPE_ID)
    {
        // Should already be open or no connection
        //pCell1->setConnection(d, CellConnection::OPEN_CONNECTION);
        //pCell2->setConnection(opposite(d), CellConnection::OPEN_CONNECTION);
        return;
    }

    if (pCell1->getConnection(d) == CellConnection::FIXTURE_CONNECTION ||
        pCell2->getConnection(opposite(d)) == CellConnection::FIXTURE_CONNECTION)
        throw PlumberException("attempt to disconnect fixture");

    // Set connection to OPEN_CONNECTOR or OPEN_FIXTURE depending upon what else is connected to the cells
    if (pCell1->getConnection(d) == CellConnection::TEMPORARY_CONNECTION ||
        pCell2->getConnection(opposite(d)) == CellConnection::TEMPORARY_CONNECTION)
    {
        pCell1->setConnection(d, CellConnection::OPEN_CONNECTION);
        pCell2->setConnection(opposite(d), CellConnection::OPEN_CONNECTION);
    }
    if (pCell1->getConnection(d) == CellConnection::OPEN_FIXTURE ||
        pCell2->getConnection(opposite(d)) == CellConnection::OPEN_FIXTURE)
    {} // nothing to disconnect
}
*/
