// searchfvs_withoutcbc.cc, written by Kazuki Maeda <kmaeda at kmaeda.net>, 2020

#include "searchfvs.hh"

using namespace std;

void (*computeminnumFVS)(vector<bitset<maxnumnodes>>& cycles,
                         const int numnodes,
                         unsigned int& minnumFVS,
                         bitset<maxnumnodes>& FVS) = NULL;
