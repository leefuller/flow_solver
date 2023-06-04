#include <iostream>

//#include "../include/Puzzle.h"
#include "../app/formations.h"
#include "test_helper.h"

bool testInvalidDeviation ()
{
    bool result = true;
    // TODO
    return result;
}

bool testEntrapment ()
{
    bool result = true;
    // TODO
    return result;
}

bool testCornerCheck ()
{
    bool result = true;
    // TODO
    return result;
}

int main ()
{
    bool result = true;
    //std::shared_ptr<LogScope> sl = logger.getScopedLogger("Test formations");
    result = result && testCornerCheck();
    result = result && testInvalidDeviation();
    result = result && testEntrapment();
    return result ? 0 : 1;
}

