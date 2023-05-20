#ifndef TEST_HELP_H
#define TEST_HELP_H

#include <iostream>

inline bool check (const char * p, bool condition)
{
    if (p != nullptr)
        std::cout << p << std::endl;
    return condition;
}

inline bool check (bool condition)
{ return check(nullptr, condition); }

#endif
