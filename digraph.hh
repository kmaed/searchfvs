// digraph.hh, written by Kazuki Maeda <kmaeda at kmaeda.net>, 2020

#ifndef _DIGRAPH_HH_
#define _DIGRAPH_HH_

#include <iostream>
#include <vector>
#include <string>
#include <bitset>
#include <algorithm>

class digraph {
public:
  const static int maxnumnodes = 10000; // The max number of nodes of input network.
                                // If the input network has more nodes, increase this value
                                // and recompile.
                                // Developer notes: we can use boost::dynamic_bitset
                                // as an alternative of std::bitset. However, dynamic_bitset
                                // is much slower than bitset.

private:
  int numnodes, numedges;
  unsigned int minnumFVS;
  std::vector<std::string> nodes;
  std::vector<std::vector<int>> edges; // edges[i] = {j0, j1, j2, ...}: i -> j
  unsigned int srtnodeind[maxnumnodes]; // node index sorted by the number appearing in the minimal FVSs
  int statFVS[maxnumnodes];
  std::vector<std::bitset<maxnumnodes>> cycles; // cycles[i][j]: if node j is in cycle i
                                                // Note: this is the set of "local minimum" cycles (chordless cycles).
                                                // E.g., if there are cycles {1101} and {1001},
                                                // only {1001} is included in the cycles variable.
  std::vector<std::string> removednodes;   // actually removed nodes
  std::vector<std::bitset<maxnumnodes>> FVSs;

  inline static void _calcstatFVS(const std::vector<std::bitset<maxnumnodes>> listFVSs,
                                  int* statFVS) {
    for(auto i = 0; i < maxnumnodes; ++i)
      statFVS[i] = count_if(listFVSs.begin(), listFVSs.end(),
                            [&](const std::bitset<maxnumnodes>& b)->bool{return b[i];});}

  void addnode(const std::string node);
  void addandreducecycles(const std::bitset<maxnumnodes>& cycle);
  void _detectcycles(const int start, const int i,
                     const std::bitset<maxnumnodes>& searched,
                     std::bitset<maxnumnodes>& path);
  void _dfs(unsigned int cyclenum,
            std::bitset<maxnumnodes>& selected,
            std::bitset<maxnumnodes>& searched);
  void _outputFVSsastree(const std::vector<std::bitset<maxnumnodes>>& currentFVSs,
                         const int* statFVS,
                         const int pnum,
                         const int level,
                         const bool printpolynomial,
                         const int maxtreedepth) const;

public:
  digraph();
  int read(const std::string filename, const std::vector<std::string>& removenodelist);
  void detectcycles();
  void calcstatFVS();
  void outputcycles(const bool nolist) const;
  void outputremovednodes() const;
  void outputstat() const;
  void outputFVSs() const;
  void outputFVSsaspolynomial() const;

  inline void dfs(){
    std::bitset<maxnumnodes> selected, searched;
    _dfs(0, selected, searched);
  }

  inline void outputheader() const {
    std::cout << "#nodes,#edges,#[nodes of minimal FVS] = " << numnodes << "," << numedges << "," << minnumFVS << std::endl;
  }

  inline void outputFVSsastree(const bool printpolynomial, const int maxtreedepth) const {
    _outputFVSsastree(FVSs, statFVS, FVSs.size(), 0, printpolynomial, maxtreedepth);
  }

  void computeminnumFVS(const bool onlycomputemin, const bool nolist);
};

#endif
