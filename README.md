# GraphRanker

## Specification

* The objective of this project is to manage a ranking of weighted direct graphs. The ranking keeps track of the k "best" graphs.
* The program to make takes a file in input containing two parameters, only once (on the first line of the file, separated by a space):
  * **d**: the number of nodes in each graph
  * **k**: the length of the ranking
* A sequence of commands: **AddGraph** and **TopK**

**AddGraph** adds a graph to the list of graphs that are taken into consideration for the ranking. It is followed by adjacency matrix of the graph, with its elements separated by commas.
The graph's nodes are logically tagged by an integer index between 0 and d-1; the node in position 0 is the node from which the outgoing arcs are described in the first row of the matrix.
The weights of each arc is represented by an integer ranging from 0 to 2^32-1.
(e.g. AddGraph 3,7,42 means the first node has an arc going to node #0 with weight 3, one to node #1 with weight 7, and one to #3 with weight 42)

**TopK** prints the indexes of the k graphs having the k lowest values of the following metric: sum of the shortest paths between node 0 and all the other nodes.
It only considers the graphs that were added before calling the command TopK.
Should there be graphs with the same score, the priority will be given to the those that were added earlier.
The k integers are printed on one row, separated by a space, in any order.

Examples and figures: see *specification.pdf*

## Usage
* Compile the program with `gcc -std=c11 -Wall -Wextra -pedantic -O2 -g3 GraphRanker.c`
* run `./a.out > <test_file.txt>`
