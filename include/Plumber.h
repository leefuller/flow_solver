#ifndef PLUMBER_H
#define PLUMBER_H

#include <memory>

#include "Pipe.h"
#include "Direction.h"
#include "Cell.h"
#include "PuzzleException.h"

class Puzzle;

class PlumberException : public PuzzleException
{
    public:
        PlumberException (const char * fmt,...) noexcept;

        PlumberException (const std::string & msg) noexcept
            : PuzzleException(msg)
        {}

        PlumberException (const std::string && msg) noexcept
            : PuzzleException(msg)
        {}

        PlumberException (const PlumberException & ex) = default;
};

/**
 * Responsible for connecting pipes and updating Cell data.
 */
class Plumber
{
    public:
        //Plumber (std::shared_ptr<Puzzle> puzzle)
        Plumber (Puzzle * p) noexcept
            : m_puzzle(p)
        {}

        void connect (Coordinate c1, Coordinate c2, PipeId idPipe, CellConnection con) const noexcept(false);

        // TODO? void disconnect (Coordinate c1, Coordinate c2) const noexcept(false);

    private:
        //std::shared_ptr<Puzzle> m_puzzle;
        Puzzle * m_puzzle;
};

#endif
