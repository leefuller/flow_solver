/*
 Puzzle definition:
 Each puzzle cell is represented using 3 horizontally separated characters,
 and 3 vertically separated, allowing walls to be defined.
 Each cell shares the character between itself and the neighbour.
 | is a vertical wall between cells
 = is a horizontal wall between cells
 A space means there is no wall between cells.
 A '.' represents an empty cell.
 A letter, or whatever other single character you like, except the special ones previously
 mentioned, represents a pipe start/end point.
 Distinction between which point gets identified as end 1 and end 2 is not made here.

 Unreachable cells have a space character instead of "." or pipe identifier.
 They need to be defined so that the entire puzzle is encapsulated within an outer wall.
*/


/* This puzzle has initially only one way to go at each endpoint for A.
 * then B, and continuing iteratively until the puzzle is actually solved in the preliminary handling.
 *
 ie. Preliminary checks prior to generating potential routes should generate, iteratively:
 |A . . . . . A|    because of the "only one way" rule applied to A
 |A B . . . B A|
 then
 |A A . . . A A|    because of the "only one way" rule applied to A and B
 |A B . . . B A|
 then
 |A A A . A A A|    because of the "only one way" rule applied to A and B
 |A B B . B B A|
 then
 |A A A A A A A|    because of the "only one way" rule applied to A and B
 |A B B B B B A|

 */
const char * simplePuzzleDef1 = \
{" = = = = = = = ,"  \
 "|. . . . . . .|,"  \
 "|             |,"  \
 "|A B . . . B A|,"  \
 " = = = = = = = "   \
};

const char * simplePuzzleDef2 = \
{" = = = = = = ,"  \
 "|. . R Y . .|,"  \
 "|           |,"  \
 "|. . B O G .|,"  \
 "|           |,"  \
 "|. . O . . .|,"  \
 "|           |,"  \
 "|R . G . . .|,"  \
 "|           |,"  \
 "|Y . . . B .|,"  \
 "|           |,"  \
 "|. . . . . .|,"  \
 " = = = = = = "   \
};

const char * courtyard1 = \
{" = = = = = = = ,"  \
 "|. . . . . . .|,"  \
 "|  ===   ===  |,"  \
 "|.|. . . . .|.|,"  \
 "| |         | |,"  \
 "|G|. Y B . .|.|,"  \
 "|             |,"  \
 "|. . . Y . . .|,"  \
 "|             |,"  \
 "|.|B . . . .|.|,"  \
 "| |         | |,"  \
 "|R|R . . . .|.|,"  \
 "|  ===   ===  |,"  \
 "|G . . . . . .|,"  \
 " = = = = = = ="   \
};

const char * pockets34 = \
{" = = = = = = = = = ,"  \
 "|. . P N . . . . .|,"  \
 "|                 |,"  \
 "|.|R|. . .|G|. . .|,"  \
 "|  =       =      |,"  \
 "|. . . . . . .|N|.|,"  \
 "|              =  |,"  \
 "|. . .|P|. . . . .|,"  \
 "|      =          |,"  \
 "|.|O|. . .|Y|. . .|,"  \
 "|  =       =      |,"  \
 "|. R . . . . .|B|.|,"  \
 "|              =  |,"  \
 "|. . . .|G|Y . . .|,"  \
 "|        =        |,"  \
 "|. .|K|. . .|K|. .|,"  \
 "|    =       =    |,"  \
 "|O . . . . . . . B|,"  \
 " = = = = = = = = = "   \
};

/*
 Applying preliminary rules should get a starting point:
                          Applying other logic for solution:
                         0 1 2 3 4 5 6 7 8
 = = = = = = = = =       = = = = = = = = =
|R R P N . G . . .|   0 |R R P N N G G G G| R:{1,1},{0,1},{0,0},{1,0},{2,0},{2,1},{2,2},{3,2},{4,2},{5,2},{5,1}
|                 |     |                 | P:{0,2},{1,2},{1,3},{2,3},{3,3}
|R|R|P . .|G|. N .|   1 |R|R|P P N|G|N N G| O:{4,1},{3,1},{3,0},{4,0},{5,0},{6,0},{7,0},{8,0}
|  =       =      |     |  =       =      | N:{0,3},{0,4},{1,4},{2,4},{2,5},{2,6},{1,6},{1,7},{2,7}
|R . . P . . .|N|.|   2 |R R R P N N N|N|G| B:{5,7},{4,7},{4,8},{5,8},{6,8},{7,8},{8,8}
|              =  |     |              =  | G:{1,5},{0,5},{0,6},{0,7},{0,8},{1,8},{2,8},{3,8},{3,7},{3,6},{4,6},{5,6},{5,5},{5,4},{6,4}
|. O .|P|. Y . . .|   3 |O O R|P|Y Y G G G| Y:{4,5},{3,5},{3,4},{4,4},{4,3},{5,3},{6,3},{7,3},{7,4},{7,5},{6,5}
|      =          |     |      =          | K:{7,2},{6,2},{6,1},{7,1},{8,1},{8,2},{8,3},{8,4},{8,5},{8,6},{8,7},{7,7},{6,7},{6,6},{7,6}
|.|O|. . .|Y|. B .|   4 |O|O|R Y Y|Y|G B B|
|  =       =      |     |  =       =      |
|. R . . G . .|B|.|   5 |O R R Y G G G|B|B|
|              =  |     |              =  |
|. . K .|G|Y K . .|   6 |O K K Y|G|Y K K B|
|        =        |     |        =        |
|. .|K|. . .|K|. .|   7 |O K|K|Y Y Y|K|K B|
|    =       =    |     |    =       =    |
|O . . . . . . . B|   8 |O K K K K K K K B|
 = = = = = = = = =       = = = = = = = = =
*/

// Just by running the "only one way" rule repeatedly,
// the puzzle is solved in the preliminary phase.
const char * puzzleDef3 = \
{" =================== ,"  \
 "|. Y|     |. . . . .|,"  \
 "|  =   =  |         |,"  \
 "|.|   |R| |p . b O .|,"  \
 "| |  =  | |=        |,"  \
 "|.| |. .|   |b . . .|,"  \
 "| | |    =| |=   =  |,"  \
 "|.| |. . O|   |.| |p|,"  \
 "| | |      =  | | | |,"  \
 "|.| |. . . G| |.| |Y|,"  \
 "| | |      =  | | | |,"  \
 "|.| |. . G|   |.| |.|,"  \
 "| | |      ===  | | |,"  \
 "|.| |. . . . . .| |.|,"  \
 "| | |    =======  | |,"  \
 "|.| |. R|         |.|,"  \
 "| |  ===   =======  |,"  \
 "|.|       |. . . . .|,"  \
 "|  =======          |,"  \
 "|. . . . . . P . . P|,"  \
 " =================== "   \
};

const char * worm31 = \
{" ===================== ,"  \
 "|N .|     |O . . . . .|,"  \
 "|   |  =   =          |,"  \
 "|G .| |B|   |P . . . .|,"  \
 "|   | |  ===          |,"  \
 "|. .| |. . . . . . . .|,"  \
 "|   | |               |,"  \
 "|. .| |B . O . b . . .|,"  \
 "|   |  =              |,"  \
 "|. .|   |. W . . . P .|,"  \
 "|    ===              |,"  \
 "|. . Y . . Y . b W . .|,"  \
 "|                     |,"  \
 "|. . . . . . . . p . N|,"  \
 "|                     |,"  \
 "|. . . p . . . . . . .|,"  \
 "|                     |,"  \
 "|. . . . . . . . . R .|,"  \
 "|                     |,"  \
 "|. . R . . . . . . . .|,"  \
 "|                     |,"  \
 "|G . . . . . . . . . .|,"  \
 " ===================== "   \
};

/*
Repetatively applying         Applying the dead end
the "only one way" rule       detection can eliminate
gets to this point.           'O' from the allowed
 =====================        to be at mark '*'
|N N|     |O O O . . .|
|   |  =   =          |
|G N| |B|   |P . . . .|
|   | |  ===          |
|G N| |B . * . . . . .|
|   | |               |
|G N| |B . O . b . . .|
|   |  =              |
|G N|   |. W . . . P .|
|    ===              |
|G N Y . . Y . b W . .|
|                     |
|G N . . . . . . p . N|
|                     |
|G . . p . . . . . . .|
|                     |
|. . . . . . . . . R .|
|                     |
|. . R . . . . . . . .|
|                     |
|G . . . . . . . . . .|
 =====================
 */

/* Apply "only one way" rule will solve this in the preliminary phase */
const char * inkblot1 = \
{" =============== ,"  \
 "|G .|       |. G|,"  \
 "|    =     =    |,"  \
 "|R . .|   |. . B|,"  \
 "|      ===      |,"  \
 "|. O . . . . Y .|,"  \
 "|    =======    |,"  \
 "|. .|       |. B|,"  \
 "|   |       |   |,"  \
 "|. .|       |. .|,"  \
 "|   |       |   |,"  \
 "|R .|       |b Y|,"  \
 "|   |       |   |,"  \
 "|. .|       |. .|,"  \
 "|  =         =  |,"  \
 "|O|           |b|,"  \
 " =============== "   \
};

/**
 * @return the puzzle definition for a single puzzle
 */
const char * getPuzzleDef ()
{
	//return simplePuzzleDef1;
    return simplePuzzleDef2;
    //return courtyard1;
	//return puzzleDef3;
    //return inkblot1;
    //return pockets34;
	//return worm31;
}
