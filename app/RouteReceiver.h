#ifndef ROUTERECEIVER_H
#define ROUTERECEIVER_H

#include "Puzzle.h"

#include <array>
#include <vector>

using Coordinate = std::array<unsigned, 2>;
using Route = std::vector<Coordinate>;

/**
 * Base class for those to process routes.
 * RouteGenerator accepts instances to callback when a route is generated.
 */
class RouteReceiver
{
  public:
    virtual void processRoute (PipeId idPipe, Route & route) = 0;

    virtual ~RouteReceiver ()
    {}
};

#endif
