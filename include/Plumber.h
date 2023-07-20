#ifndef PLUMBER_H
#define PLUMBER_H

#include <memory>
#include <string>

#include "Pipe.h"
#include "Direction.h"
#include "Cell.h"
#include "PuzzleException.h"

class Puzzle;

class PlumberException : public PuzzleException
{
    public:
        PlumberException (const SourceRef & ref, const std::string & msg) noexcept
            : PuzzleException(ref, msg)
        {}

        PlumberException (const SourceRef & ref, const std::string && msg) noexcept
            : PuzzleException(ref, msg)
        {}

        PlumberException (const SourceRef & ref, const std::string & msg, const Coordinate & c) noexcept
            : PuzzleException(ref, msg + " at " + std::to_string(c[0]) + "," + std::to_string(c[1]))
        {}

        PlumberException (const PlumberException & ex) = default;
};

/**
 * Responsible for connecting pipes and updating Cell data.
 */
class Plumber
{
    public:
        //Plumber (PuzzlePtr puzzle)
        Plumber (Puzzle * p) noexcept
            : m_puzzle(p)
        {}

        void connect (Coordinate c1, Coordinate c2, PipeId idPipe, CellConnection con) const noexcept(false);

        // TODO? void disconnect (Coordinate c1, Coordinate c2) const noexcept(false);

        bool removeConnector (CellPtr pCell, Direction d) noexcept(false);

    private:

        //PuzzlePtr m_puzzle;
        Puzzle * m_puzzle;
};

#endif
