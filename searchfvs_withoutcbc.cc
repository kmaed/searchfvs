// searchfvs_orig.cc, written by Kazuki Maeda <kmaeda at kmaeda.net>, 2020

#include "searchfvs.hh"

using namespace std;

void (*calcminnumFVS)(vector<bitset<maxnumnodes>>& cycles,
                      const int numnodes,
                      unsigned int& minnumFVS,
                      bitset<maxnumnodes>& FVS) = NULL;
