#ifndef CELL_H
#define CELL_H

#include "Pipe.h"
#include "Direction.h"

#include <tuple>
#include <array>
#include <set>
#include <memory>
#include <algorithm>

/**
 * CellBorder declares what is at the side of the cell, disregarding pipes.
 */
enum CellBorder
{
    OPEN = 0,
    WALL = 1
};

const std::array<const char *, 2> borderStr = {"open", "wall"};

// Cell types labeled according to borders north/up, south/down, west/left, east/right
const std::array<CellBorder, 4> openBorders = {OPEN, OPEN, OPEN, OPEN};
const std::array<CellBorder, 4> horizontalChannel = {WALL, WALL, OPEN, OPEN};
const std::array<CellBorder, 4> verticalChannel = {OPEN, OPEN, WALL, WALL};
const std::array<CellBorder, 4> upperLeftCorner = {WALL, OPEN, WALL, OPEN};
const std::array<CellBorder, 4> upperRightCorner = {WALL, OPEN, OPEN, WALL};
const std::array<CellBorder, 4> lowerLeftCorner = {OPEN, WALL, WALL, OPEN};
const std::array<CellBorder, 4> lowerRightCorner = {OPEN, WALL, OPEN, WALL};

/** @return border type opposite of given border. ie from OPEN to WALL, or from WALL to OPEN. */
inline CellBorder toggleBorder (const CellBorder & border) noexcept
{ return border == CellBorder::OPEN ? CellBorder::WALL : CellBorder::OPEN; }

/** @return border that is the opposite of the given one. */
inline const std::array<CellBorder, 4> getOppositeBorder (const std::array<CellBorder, 4> & border) noexcept
{ return std::array<CellBorder, 4>{ toggleBorder(border[0]), toggleBorder(border[1]), toggleBorder(border[2]), toggleBorder(border[3]) }; }

// Each cell has a connection possibility per Direction
enum CellConnection
{
    NO_CONNECTOR,         // For a side against a wall, or against a different pipe that is a fixture
    OPEN_CONNECTOR,       // For an available connector, whether to an empty cell or non-fixture pipe
    TEMPORARY_CONNECTION, // A pipe is connected, but not identified as a fixture yet
    FIXTURE_CONNECTION,   // A permanent connection between 2 cells
    OPEN_FIXTURE          // An open connector to a fixture. Can only occur at the end of a fixture that is not a pipe endpoint.
    // Correspondingly:
    // There is at least 1 free connector at a fixture endpoint, unless the endpoint is an endpoint of the entire pipe.
    // An intermediate fixture (having a FIXTURE_CONNECTION on 2 sides) has no free connectors.
};

const std::array<CellConnection, 4> noConnections = {NO_CONNECTOR, NO_CONNECTOR, NO_CONNECTOR, NO_CONNECTOR};

const std::array<const char *, 5> connectionStr = {"no connector", "open connector", "temporary connection", "fixture connection", "open fixture"};

class Cell;
using CellPtr = std::shared_ptr<Cell>;
using ConstCellPtr = std::shared_ptr<const Cell>;

#define USE_PIPE_POSSIBILITY 1

/**
 * Represent a single cell in a puzzle.
 * Intended as a data holder. Should contain little to no logic.
 */
class Cell
{
  private:
    Cell () = default;
  public:
    ~Cell () = default;
    Cell (const Cell&) = default;
    Cell (const Cell&&) = delete;
    Cell operator= (const Cell&) const = delete;
    Cell operator= (const Cell&&) = delete;

    explicit Cell (const std::unique_ptr<Cell>& p)
        : Cell(*(p.get()))
    {}

    /**
     * Set what is contained in the cell.
     * @param idPipe    Pipe identifier, or NO_PIPE_ID
     * @param permanent     If true, changes the set of possibilities to only contain the given identifier.
     */
    void setPipeId (PipeId idPipe, bool permanent = false) noexcept
    {
        m_idPipe = idPipe;
#if USE_PIPE_POSSIBILITY
        if (permanent)
            setPossiblePipes(idPipe);
#endif
    }

    /**
     * Set what is contained in the cell, and remove other possible ids.
     * @param idPipe    Pipe identifier
     */
    void setPipeIdPermanent (PipeId idPipe) noexcept
    { setPipeId(idPipe, true); }

    /**
     * @return cell coordinate.
     */
    Coordinate getCoordinate () const noexcept
    { return m_coordinate; }

    /**
     * @return what is contained in the cell.
     */
    PipeId getPipeId () const noexcept
    { return m_idPipe; }

    /**
     * @return true if the cell is empty.
     */
    bool isEmpty () const noexcept
    { return getPipeId() == NO_PIPE_ID; }

    /**
     * Check whether cell is a fixture.
     * A fixture is an endpoint, or a cell where any connection is fixed.
     */
    bool isFixture () const noexcept
    {
        if (isEndpoint())
            return true;
        for (Direction d : allTraversalDirections)
        {
            CellConnection c = getConnection(d);
            if (c == CellConnection::FIXTURE_CONNECTION || c == CellConnection::OPEN_FIXTURE)
                return true;
        }
        return false;
    }

    PipeEnd getEndpoint () const noexcept
    { return m_endpoint; }

    CellConnection getConnection (Direction d) const noexcept
    { return m_connection[d]; }

    bool isConnected (Direction d) const noexcept
    { return m_connection[d] == CellConnection::TEMPORARY_CONNECTION
          || m_connection[d] == CellConnection::FIXTURE_CONNECTION; }

    /**
     * @return true if the cell is an endpoint.
     */
    bool isEndpoint () const noexcept
    { return getEndpoint() != PipeEnd::NO_ENDPOINT; }

    /**
     * Get the border of the cell in the given direction.
     */
    CellBorder getBorder (Direction d) const noexcept
    { return d == Direction::NONE ? CellBorder::OPEN : m_border[d]; }

    /** @return all the borders of the cell */
    const std::array<CellBorder, 4> getBorders () const noexcept
    { return m_border; }

    std::array<CellConnection, 4> getConnections () const noexcept
    { return m_connection; }

    /** @return the number of walls that the cell has */
    unsigned countWalls () const noexcept
    { return std::count_if(std::begin(m_border), std::end(m_border), [](CellBorder b){ return b == CellBorder::WALL; }); }

    /** @return the number of fixed connections that the cell has */
    unsigned countFixtureConnections () const noexcept
    { return std::count_if(std::begin(m_connection), std::end(m_connection), [](CellConnection c){ return c == CellConnection::FIXTURE_CONNECTION; }); }

    bool canAcceptConnection (Direction from) const noexcept;

    /**
     * @return true if the cell border is defined as open in the given direction.
     * NOTE a border does not change once defined.
     * ie. A pipe running adjacent to that border does not change it from OPEN.
     */
    bool isBorderOpen (Direction d) const noexcept
    { return getBorder(d) == OPEN; }

    /**
     * Considering only borders (not pipes), determine if the cell is only traversable
     * both left and right.
     * @return true if the cell borders mean the traversable directions are only left and right.
     */
    bool isHorizontalChannel () const noexcept
    { return getBorders() == horizontalChannel; }

    /**
     * Considering only borders (not pipes), determine if the cell is only traversable
     * both up and down.
     * @return true if the cell borders mean the traversable directions are only up and down.
     */
    bool isVerticalChannel () const noexcept
    { return getBorders() == verticalChannel; }

    /**
     * Considering only borders (not pipes), determine if the cell is only traversable
     * as a corner.
     * @return true if the cell borders mean the traversable directions are at 90 degrees to each other.
     */
    bool isCorner () const noexcept
    { return countWalls() == 2 && !isVerticalChannel() && !isHorizontalChannel(); }

    bool operator== (const Cell &) const noexcept;

    std::string toString () const noexcept;

    std::ostream & describe (std::ostream & os) const noexcept;

    static void setOutputConnectorRep (bool repr = true) noexcept;

    static bool isOutputConnectorRep () noexcept
    { return outputConnectorRep; }

#if USE_PIPE_POSSIBILITY
    /** Add the given pipe identifier to list of possibilities for this cell. */
    void addPossibility (PipeId id) noexcept
    { m_possiblePipes.insert(id); }

    /** Remove the given pipe identifier to list of possibilities for this cell. */
    void removePossibility (PipeId id) noexcept
    { m_possiblePipes.erase(id); }

    /** Set list of possible pipes. */
    void setPossiblePipes (const std::set<PipeId> & s) noexcept
    { m_possiblePipes = s; }

    /** Set list of possible pipes. */
    void setPossiblePipes (PipeId id) noexcept
    {
        m_possiblePipes.clear();
        addPossibility(id);
    }

    /** Get possible pipes. */
    const std::set<PipeId> & getPossiblePipes () const noexcept
    { return m_possiblePipes; }

    /*bool hasPossible (PipeId id) const noexcept
    { return std::find(std::begin(m_possiblePipes), std::end(m_possiblePipes), id) != m_possibilePipes.end(); }*/
#endif

  private:

    static CellPtr createCell (Coordinate c, PipeId idPipe) noexcept;

    /** Whether or not output to stream includes connector representation */
    static bool outputConnectorRep;

    /**
     * Set the border of the cell in the given direction.
     */
    void setBorder (Direction d, CellBorder b) noexcept
    { m_border[d] = b; }

    void setEndpoint (PipeEnd end) noexcept
    { m_endpoint = end; }

    /**
     * Set the cell coordinates.
     * (Private in order to maintain integrity in the context of a puzzle.)
     */
    void setCoordinate (Coordinate c) noexcept
    { m_coordinate = c; }

    /**
     * Set the connection status for the given direction.
     */
    void setConnection (Direction d, CellConnection c) noexcept
    { m_connection[d] = c; }

    /**
     * Set the connection status for all directions.
     */
    void setConnections (std::array<CellConnection, 4> c) noexcept
    { m_connection = c; }

    void changeConnections (CellConnection from, CellConnection to) noexcept(false);

    /** Coordinate of cell */
    Coordinate m_coordinate;

#if USE_PIPE_POSSIBILITY
    /** For potential external logic to keep a set of possible pipe identifiers */
    std::set<PipeId> m_possiblePipes;
#endif

    PipeId m_idPipe{NO_PIPE_ID};

    /** Refer to enum for endpoints */
    PipeEnd m_endpoint{PipeEnd::NO_ENDPOINT};

    /**
     * The type of connection in each direction from the cell.
     */
    std::array<CellConnection, 4> m_connection{OPEN_CONNECTOR, OPEN_CONNECTOR, OPEN_CONNECTOR, OPEN_CONNECTOR};

    /**
     * Defines the state of each border of the cell.
     * A border does not change once defined.
     * ie. A pipe running adjacent to that border does not change it from OPEN.
     */
    std::array<CellBorder, 4> m_border{OPEN, OPEN, OPEN, OPEN};

    friend class PuzzleDefinition; // Let the puzzle generator assign some private attributes
    friend class Plumber; // To tinker with the pipes that are private
};

std::ostream & operator<< (std::ostream & os, const Cell & cell) noexcept;
std::ostream & operator<< (std::ostream & os, const CellPtr) noexcept;
std::ostream & operator<< (std::ostream & os, const std::vector<Cell> & row) noexcept;
std::ostream & operator<< (std::ostream & os, const std::vector<CellPtr> & row) noexcept;

#endif
