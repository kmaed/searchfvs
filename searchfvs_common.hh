// searchfvs_common.hh, written by Kazuki Maeda <kmaeda at kmaeda.net>, 2020

#ifndef _SEARCHFVS_COMMON_HH_
#define _SEARCHFVS_COMMON_HH_

#include<bitset>
#include<vector>

const static int maxnumnodes = 10000; // The max number of nodes of input network.
                                      // If the input network has more nodes, increase this value
                                      // and recompile.
                                      // Developer notes: we can use boost::dynamic_bitset
                                      // as an alternative of std::bitset. However, dynamic_bitset
                                      // is much slower than bitset.

void search(std::vector<std::bitset<maxnumnodes>>& cycles,
            const int numnodes,
            std::vector<std::bitset<maxnumnodes>>& FVSs,
            unsigned int& minnumFVS);

#endif
