#include "Solver.h"
#include "../include/Logger.h"

#include <locale>
/** To separate thousands by a comma in outputting a number */
class comma_numpunct : public std::numpunct<char>
{
  protected:
    virtual char do_thousands_sep() const override
    { return ','; }

    virtual std::string do_grouping() const override
    { return "\03"; }
};

extern const char * getPuzzleDef ();
extern const char * puzzleDefs[];

static Logger & logger = Logger::getDefaultLogger();

int main (int argc, const char * argv[])
{
    // Setup locale to separate thousands in numbers
    std::locale comma_locale(std::locale(), new comma_numpunct());
    // tell cout to use new locale.
    std::cout.imbue(comma_locale);

    try
    {
        for (unsigned i = 0; puzzleDefs[i] != nullptr; ++i)
        {
            Solver solver(puzzleDefs[i]);
            bool solved = solver.solve();
            if (!solved)
                logger << "Solution not found" << std::endl;
        }
    }
    catch (const PuzzleException & ex)
    {
    	logger << ex << std::endl;
    }
    catch (const std::exception & ex)
    {
        logger << ex.what() << std::endl;
    }
    catch (const char * s)
    {
        logger << s << std::endl;
    }
    catch (...)
    {
        logger << "Unexpected object thrown" << std::endl;
    }

    logger << "Exit." << std::endl;
    return 0;
}
