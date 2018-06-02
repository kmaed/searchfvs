# Searchfvs

Searchfvs is a simple solver for the directed [feedback vertex set (FVS) problem](https://en.wikipedia.org/wiki/Feedback_vertex_set).

## Compile

The program is written in C++11. You must prepare a C++ compiler
implemented C++11 features. E.g., recent g++ will generate the binary by
the following command:

    $ g++ -O2 -std=c++11 -o searchfvs searchfvs.cc

## Usage

    $ ./searchfvs [options] <network data>

&lt;network data&gt; should be a text file containing the list of edges of a network
(see the next section). Then the program finds all the minimal FVSs of
the network. The program outputs first the number of nodes of minimal FVSs,
and the names of the nodes in each minimal FVS with running number.

For example, the file [ciona.txt](https://github.com/kmaed/searchfvs/blob/master/ciona.txt) is a gene regulatory network data of Ciona embryos.

    $ ./searchfvs ciona.txt

will give the list of minimal FVSs of the network.

One can specify several options. `--help` outputs the list of options.

## Network data

For example, the line

    ABC DE

means that there is an edge from a node "ABC" to a node "DE" in the network.
Since space characters are used as delimiters, the names of nodes should not
include space characters. If there is a # character at the begin of a line,
the program ignores the line.

## Contact

Kazuki Maeda "kmaeda at kmaeda.net"