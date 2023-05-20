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
    PIPE_END_1 = 1,
    PIPE_END_2 = 2,
    //PIPE_INTERMEDIATE_FIXTURE // TODO A fixture that is not an end point
};

using PipeId=char;

#define NO_PIPE_ID      0

#endif
