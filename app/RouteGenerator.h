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
    virtual void generateRoutes (PipeId id, std::shared_ptr<const Puzzle> puzzle) = 0;

    virtual ~RouteGenerator () {}

    inline void emitRoute (PipeId idPipe, Route & route) const noexcept
    {
    	for (RouteReceiver * rx : m_receivers)
        {
            try
            {
                rx->processRoute(idPipe, route);
            }
            catch (const PuzzleException & ex)
            {
                std::cerr << "Puzzle exception in route processor: " << ex << std::endl;
            }
            catch (...)
            {
                std::cerr << "Something thrown by route processor" << std::endl;
            }
        }
    }

    void addReceiver(RouteReceiver * receiver)
    {
        m_receivers.push_back(receiver);
    }

    void removeReceiver(RouteReceiver * receiver)
    {
        m_receivers.remove(receiver);
    }

  private:
    std::list<RouteReceiver *> m_receivers;
};

#endif
