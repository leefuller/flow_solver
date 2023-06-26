#ifndef ROUTERECEIVER_H
#define ROUTERECEIVER_H

#include "Puzzle.h"

#include <array>
#include <vector>

using Coordinate = std::array<int, 2>;
using Route = std::vector<Coordinate>;

/**
 * Base class for those to process routes.
 */
class RouteReceiver
{
  public:
    /**
     * @param idPipe    Identifier of pipe for route
     * @param route     Route to handle
     * @return false to indicate route generation can stop, where the source is a RouteGenerator
     */
    virtual bool processRoute (PipeId idPipe, Route & route) = 0;

    virtual ~RouteReceiver ()
    {}
};

#endif
