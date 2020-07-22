// searchfvs_orig.cc, written by Kazuki Maeda <kmaeda at kmaeda.net>, 2017-2020

#include <iostream>
#include "searchfvs.hh"

using namespace std;


void search(vector<bitset<maxnumnodes>>& cycles,
                   const int numnodes,
                   vector<bitset<maxnumnodes>>& FVSs,
                   unsigned int& minnumFVS){
  bitset<maxnumnodes> selected, searched;
  search_rec(cycles, numnodes, FVSs, minnumFVS, 0, selected, searched);
}
