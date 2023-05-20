Solve flow puzzles

A flow puzzle contains any number of square "cells" via which "pipes" can "flow".
Initially the puzzle has mostly empty cells, with each of any number of uniquely identified pipes
having 2 end points located somewhere within the puzzle.
The puzzle is encapsulated by walls, and there can also be walls within.
There can be no flow through walls.
There is no diagonal flow.

The solution to the puzzle requires a continuous flow for each pipe between the end points for it.
Rules are:
- No flows can intersect.
- The solution uses every cell of the puzzle.
If either is not possible, the puzzle definition is invalid.

The challenge is to solve puzzles, as much as possible, before resorting to brute force.

Based on things I noticed while solving them, I have written some algorithms that 
reduce the amount of brute force necessary, but especially where puzzles are non-trivial,
resorting to brute force to complete the solution ultimately becomes necessary.

If anyone can do this using artificial intelligence, I would like to know.

