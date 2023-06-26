#ifndef ROUTEGENERATOR_H
#define ROUTEGENERATOR_H

#include <list>
#include <map>
#include <memory>
#include <iostream>

#include "Puzzle.h"
#include "RouteReceiver.h"
#include "PuzzleException.h"

class Puzzle;

/**
 * Base class for generators of routes, derived from given puzzle.
 */
class RouteGenerator
{
  public:
    virtual void generateRoutes (PipeId id, ConstPuzzlePtr puzzle) = 0;

    virtual ~RouteGenerator () {}

    /**
     * Callback from the Graph route generator when a completed route is found.
     * @param idPipe    Identifier of pipe for route
     * @param route     Route to handle
     * @return false to indicate route generation can stop
     */
    inline bool emitRoute (PipeId idPipe, Route & route) const noexcept
    {
        if (m_receiver == nullptr)
            return false; // no point generating routes if there is no handler for them

        try
        {
            return m_receiver->processRoute(idPipe, route);
        }
        catch (const PuzzleException & ex)
        {
            std::cerr << "Puzzle exception in route processor: " << ex << std::endl;
        }
        catch (...)
        {
            std::cerr << "Something thrown by route processor" << std::endl;
        }
        return true;
    }

    void setReceiver(RouteReceiver * receiver)
    { m_receiver = receiver; }

    void removeReceiver()
    { m_receiver = nullptr; }

  private:
    RouteReceiver * m_receiver{nullptr};
};

#endif
