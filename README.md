# Searchfvs

Searchfvs is a simple solver for the directed [feedback vertex set (FVS) problem](https://en.wikipedia.org/wiki/Feedback_vertex_set).

## Compile

The program is written in C++. You must prepare a C++ compiler implemented C++14 features.
Type

    $ make searchfvs

Then the binary `searchfvs` will be generated. This binary does not depend on
any external libraries.

If [COIN-OR CBC solver](https://github.com/coin-or/Cbc) is installed on your computer,
type

    $ make searchfvs_withcbc

or simply

    $ make

Then the binary `searchfvs_withcbc` will be generated.
This version is a bit faster than the &quot;without CBC&quot; version
thanks to finding a solution of a set cover problem as ILP by CBC solver.

## Usage

    $ ./searchfvs [options] <network data>
    $ ./searchfvs_withcbc [options] <network data>

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

## Related works

* [Kenji Kobayashi, Kazuki Maeda, Miki Tokuoka, Atsushi Mochizuki and Yutaka Satou, Controlling cell fate specification system by key genes determined from network structure, iScience 4 (2018) 281–293](https://doi.org/10.1016/j.isci.2018.05.004).

## Contact

Kazuki Maeda "kmaeda at kmaeda.net"
