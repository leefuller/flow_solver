#ifndef PIPE_H
#define PIPE_H

/**
 * A pipe has 2 ends that must be identified separately, so that a solution connects both endpoints.
 * Also, some puzzles can contain cells where it is obvious that only a certain pipe can pass through.
 * Identifying them as fixtures ("waypoints") could reduce the amount of processing required for a puzzle solution.
 */
enum PipeEnd
{
    NO_ENDPOINT = 0,
    PIPE_START = 1,
    PIPE_END = 2,
    //PIPE_END_PROXY,   // When a partial route is attached to PIPE_END, generated routes from pipe start are to the proxy for pipe end
    //PIPE_INTERMEDIATE_FIXTURE // TODO A fixture that is not an end point
};

#include <stdexcept>

inline PipeEnd oppositeEnd (PipeEnd e)
{
    switch (e)
    {
        case NO_ENDPOINT: break;
        case PIPE_START:  return  PIPE_END;
        case PIPE_END:    return PIPE_START;
    }
    throw std::invalid_argument("invalid endpoint parameter");
}

using PipeId=char;

#define NO_PIPE_ID      0

#endif
