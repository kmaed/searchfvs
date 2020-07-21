// searchfvs_orig.cc, written by Kazuki Maeda <kmaeda at kmaeda.net>, 2020

#include <bitset>
#include "searchfvs_common.hh"

using namespace std;

void search_body(vector<bitset<maxnumnodes>>& cycles,
                 const int numnodes,
                 vector<bitset<maxnumnodes>>& FVSs,
                 unsigned int& minnumFVS,
                 unsigned int cyclenum,
                 bitset<maxnumnodes>& selected,
                 bitset<maxnumnodes>& searched) {
  bitset<maxnumnodes> nextsearched = searched;
  while(cyclenum < cycles.size()){
    if(!(selected & cycles[cyclenum]).count()){
      if(selected.count() == minnumFVS)
        return;
      for(int i = 0; i < numnodes; ++i)
        if(cycles[cyclenum][i] && !nextsearched[i]){
          selected.set(i);
          nextsearched.set(i);   // prevent duplicate listing
          search_body(cycles, numnodes, FVSs, minnumFVS, cyclenum+1, selected, nextsearched);
          selected.reset(i);
        }
      return;
    } else
      ++cyclenum;
  }
  if(selected.count() < minnumFVS){
    minnumFVS = selected.count();
    FVSs.clear();
  }
  FVSs.push_back(selected);
}

void search(vector<bitset<maxnumnodes>>& cycles,
            const int numnodes,
            vector<bitset<maxnumnodes>>& FVSs,
            unsigned int& minnumFVS){
  bitset<maxnumnodes> selected, searched;
  search_body(cycles, numnodes, FVSs, minnumFVS, 0, selected, searched);
}
