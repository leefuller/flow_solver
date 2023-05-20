// Temporary? Experimental stuff
/**
 * Determine the position of the edge that results from moving it in a new direction.
 *                   U
 *              ------------
 *              |          |
 *          .-> | B      A | <-.
 *         v    |    O     |    v
 *    --------------------------------
 *   |     A    |          |    B     |
 * L |        O |          | O        | R
 *   |     B    |          |    A     |
 *    --------------------------------
 *         ^    |    O     |    ^
 *           -> | A      B | <-
 *              |          |
 *              ------------
 *                   D
 *
 * If right, and change to up, then       |  If up and change to right, then
 * that was down, is now right;           |  that was down, is now left;
 * that was up, is now left;              |  that was up, is now right;
 * that was right, is now up;             |  that was right, is now down;
 * that was left, is now down;            |  that was left, is now up;
 *
 * If left, and change to up, then        |  If up, and change to left, then
 * that was down, is now left;            |  that was down, is now right;
 * that was up, is now right;             |  that was up, is now left;
 * that was right, is now down;           |  that was right, is now up;
 * that was left, is now up;              |  that was left, is now down;
 *
 * If right, and change to down, then     |  If down and change to right, then
 * that was down, is now left             |  that was down, is now right
 * that was up, is now right              |  that was up, is now left
 * that was right, is now down            |  that was right, is now up
 * that was left, is now up               |  that was left, is now down
 *
 * If left, and change to down, then      |  If down and change to left, then
 * that was down, is now right            |  that was down, is now left
 * that was up, is now left               |  that was up, is now right
 * that was right, is now up              |  that was right, is now down
 * that was left, is now down             |  that was left, is now up
 *
 * Change of opposites results in opposites.
 */
// TODO maybe
Direction relativeEquivalentPosition (Direction start, Direction newDirection)
{
    if (newDirection == NONE)
        return start;
    if (newDirection == start)
        return start;
    if (newDirection == opposite(start))
        return opposite(start);

    switch (start)
    {
        case UP:    // -------------- Starting UP
            // Continuation, opposite, or NONE are already dealt with.
            // Only need to handle LEFT and RIGHT
            return (newDirection == Direction::LEFT) ? RIGHT : LEFT;
            /*switch (newDirection)
            {
                case UP:
                case DOWN:
                case LEFT:
                case RIGHT:
            }*/
        case DOWN:  // -------------- Starting DOWN
            // Continuation, opposite, or NONE are already dealt with.
            // Only need to handle LEFT and RIGHT
            return (newDirection == Direction::LEFT) ? RIGHT : LEFT;
        case LEFT:  // -------------- Starting LEFT
            // Continuation, opposite, or NONE are already dealt with.
            // Only need to handle UP and DOWN
            //return (newDirection == Direction::UP) ? UP : ;
        case RIGHT: // -------------- Starting RIGHT
            // Continuation, opposite, or NONE are already dealt with.
            // Only need to handle UP and DOWN
            return (newDirection == Direction::UP) ? LEFT : RIGHT;
        case NONE:// -------------- Starting NONE
            return newDirection;
    }
    [[unlikely]]
    throw std::invalid_argument("invalid argument");
}



//RelativeDirection getChange (Direction start, Direction newDirection)
Direction getChange (Direction start, Direction newDirection)
{
    switch (start)
    {
        case UP:    // -------------- Starting UP
            switch (newDirection)
            {
                case UP:    return UP;
                case DOWN:  return DOWN;
                case LEFT:  return LEFT;
                case RIGHT: return RIGHT;
                case NONE:  return start;
            }
            break;
        case DOWN:  // -------------- Starting DOWN
            switch (newDirection)
            {
                case UP:    return UP;
                case DOWN:  return DOWN;
                case LEFT:  return RIGHT;
                case RIGHT: return LEFT;
                case NONE:  return start;
            }
            break;
        case LEFT:  // -------------- Starting LEFT
            switch (newDirection)
            {
                case UP:    return UP;
                case DOWN:  return DOWN;
                case LEFT:  return LEFT;
                case RIGHT: return RIGHT;
                case NONE:  return start;
            }
            break;
        case RIGHT: // -------------- Starting RIGHT
            switch (newDirection)
            {
                case UP:    return UP;
                case DOWN:  return DOWN;
                case LEFT:  return LEFT;
                case RIGHT: return RIGHT;
                case NONE:  return start;
            }
            break;
        case NONE:// -------------- Starting NONE
            return newDirection;
    }
}

/**
 * Get the resulting direction from rotating in the relative direction from the start direction,
 * using 4 directions, with reference being UP.
 * For example:
 *  If you start UP, and turn LEFT, you are facing LEFT.
 *  If you start LEFT, and turn LEFT, you are facing DOWN.
 *
Direction rotate (Direction start, RelativeDirection relative)
{
    if (relative == RelativeDirection::CONTINUE)
        return start;

    switch (start)
    {
        case UP:
            switch (relative)
            {
                case CONTINUE:  return start;
                case TURN_LEFT: return Direction::LEFT;
                case TURN_RIGHT:return Direction::RIGHT;
                case OPPOSITE:  return Direction::DOWN;
                case STOP:      return Direction::NONE;
            }
            break;
        case DOWN:
            switch (relative)
            {
                case CONTINUE:  return start;
                case TURN_LEFT: return Direction::RIGHT;
                case TURN_RIGHT:return Direction::LEFT;
                case OPPOSITE:  return Direction::UP;
                case STOP:      return Direction::NONE;
            }
            break;
        case LEFT:
            switch (relative)
            {
                case CONTINUE:  return start;
                case TURN_LEFT: return Direction::DOWN;
                case TURN_RIGHT:return Direction::UP;
                case OPPOSITE:  return Direction::RIGHT;
                case STOP:      return Direction::NONE;
            }
            break;
        case RIGHT:
            switch (relative)
            {
                case CONTINUE:  return start;
                case TURN_LEFT: return Direction::UP;
                case TURN_RIGHT:return Direction::DOWN;
                case OPPOSITE:  return Direction::LEFT;
                case STOP:      return Direction::NONE;
            }
            break;
        case NONE:
            switch (relative)
            {
                case CONTINUE:  return start;
                case TURN_LEFT: return Direction::LEFT;
                case TURN_RIGHT:return Direction::RIGHT;
                case OPPOSITE:  return Direction::NONE;
                case STOP:      return Direction::NONE;
            }
            break;
    }
    [[unlikely]]
    throw std::invalid_argument("invalid argument");
}*/

/**
 * Get the resulting direction from rotating in the relative direction from the start direction,
 * using 4 directions, with reference being UP.
 * For example:
 *  If you start UP, and turn LEFT, you are facing LEFT.
 *  If you start LEFT, and turn LEFT, you are facing DOWN.
 *
Direction rotate (Direction start, Direction relative)
{
    if (relative == NONE)
        return start;
    //if (start == NONE)

    switch (start)
    {
        case UP:    // -------------- Starting UP
            switch (relative)
            {
                case UP:    return UP;
                case DOWN:  return DOWN;
                case LEFT:  return LEFT;
                case RIGHT: return RIGHT;
                case NONE:  return start;
            }
            break;
        case DOWN:  // -------------- Starting DOWN
            switch (relative)
            {
                case UP:    return UP;
                case DOWN:  return DOWN;
                case LEFT:  return RIGHT;
                case RIGHT: return LEFT;
                case NONE:  return start;
            }
            break;
        case LEFT:  // -------------- Starting LEFT
            switch (relative)
            {
                case UP:    return UP;
                case DOWN:  return DOWN;
                case LEFT:  return LEFT;
                case RIGHT: return RIGHT;
                case NONE:  return start;
            }
            break;
        case RIGHT: // -------------- Starting RIGHT
            switch (relative)
            {
                case UP:    return UP;
                case DOWN:  return DOWN;
                case LEFT:  return LEFT;
                case RIGHT: return RIGHT;
                case NONE:  return start;
            }
            break;
        case NONE:// -------------- Starting NONE
            return relative;
    }
    [[unlikely]]
    throw std::invalid_argument("invalid argument");
}*/
