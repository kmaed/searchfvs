// searchfvs.cc, written by Kazuki Maeda <kmaeda at kmaeda.net>, 2017

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <bitset>
#include <algorithm>
#include <errno.h>

using namespace std;

const int maxnumnodes = 320;    // the max number of nodes of input network
                                // If the input network has more nodes,
                                // increase this value and recompile.

int numnodes;
vector<string> nodes;

vector<vector<int>> edges;      // edges[i] = {j0, j1, j2, ...}: i -> j

vector<bitset<maxnumnodes>> cycles; // cycles[i][j]: if node j is in cycle i
                                    // Note: this is "reduced" set.
                                    // E.g., if there are cycles {1101} and {1001},
                                    // only {1001} is included in the cycles variable.
vector<bitset<maxnumnodes>> fvs;
unsigned int minfvs = maxnumnodes;

bitset<maxnumnodes> path;       // for detectcycle()

class bitset_cmp{
public:
  bool operator() (const bitset<maxnumnodes> &__x, const bitset<maxnumnodes> &__y) const {
    return __x.count() < __y.count();
  }
};

void addandreducecycles(bitset<maxnumnodes>& cycle){
  // add?
  for(auto it = cycles.begin(); it != cycles.end(); ++it)
    if((cycle | *it) == cycle)
      return;

  // reduce
  for(auto it = cycles.begin(); it != cycles.end(); ){
    if((cycle | *it) == *it)
      it = cycles.erase(it);
    else
      ++it;
  }
  cycles.push_back(cycle);
}

void detectcycle(int start, int i, bitset<maxnumnodes>& searched){
  path.set(i);
  bitset<maxnumnodes> nextsearched = searched;
  for(auto it = edges[i].begin(); it != edges[i].end(); ++it)
    nextsearched.set(*it);      // prevent redundant search
                                // E.g., if you have already found the path 0 -> 1,
                                // then you can omit the path 0 -> 2 -> 1.
  bool endflag = false;
  bool cycleflag = false;
  for(auto it = edges[i].begin(); it != edges[i].end(); ++it){
    if(*it == start){
      cycleflag = true;
      break;
    } else if(path[*it]){
      endflag = true; // If a branch returns to the node already passed,
                      // all the results of further search will redundant cycles.
    }
  }

  if(cycleflag){
    if(path.count())
      addandreducecycles(path);
  } else if(!endflag) {
    for(auto it = edges[i].begin(); it != edges[i].end(); ++it){
      if(*it < start || searched[*it])
        continue;
      else
        detectcycle(start, *it, nextsearched);
    }
  }
  path.reset(i);
}

void search(bitset<maxnumnodes>& selected, unsigned int cyclenum, bitset<maxnumnodes>& searched){
  bitset<maxnumnodes> nextsearched = searched;
  while(cyclenum < cycles.size()){
    if(!(selected & cycles[cyclenum]).count()){
      if(selected.count() == minfvs)
        return;
      for(int i = 0; i < numnodes; ++i)
        if(cycles[cyclenum][i] && !nextsearched[i]){
          selected.set(i);
          nextsearched.set(i);   // prevent duplicate listing
          search(selected, cyclenum+1, nextsearched);
          selected.reset(i);
        }
      return;
    } else
      ++cyclenum;
  }
  if(selected.count() < minfvs){
    minfvs = selected.count();
    fvs.clear();
  }
  fvs.push_back(selected);
}

void addnode(string node){
  nodes.push_back(node);
  edges.push_back(vector<int>());
  ++numnodes;
  if(numnodes > maxnumnodes){
    cerr << "Error: the number of nodes in the input network is too large!" << endl;
    cerr << "       Increase the value of maxnumnodes and recompile the program." << endl;
    exit(1);
  }
}

int main(int argc, char** argv){
  if(argc < 2){
    cerr << "Usage: " << argv[0] << " <network file>" << endl;
    return 1;
  }

  // init
  numnodes = 0;
  nodes = vector<string>();
  edges = vector<vector<int>>();

  // read
  ifstream fin;
  fin.exceptions(ios::failbit);
  try{
    fin.open(argv[1]);
  } catch (exception e){
    cerr << argv[0] << ": " << argv[1] << ": " << strerror(errno) << endl;
    return 1;
  }

  int lineno = 0;
  while(!fin.eof()){
    try{
      string tmp;
      getline(fin, tmp);
      ++lineno;
      if(tmp[0] != '#'){
        string from, to, ill;
        stringstream ss(tmp);
        ss >> from >> to >> ill;
        if(from == "" || to == "" || ill != ""){
          cerr << argv[1] << ":" << lineno << ": warning: illegal input line \"" << tmp << "\", ignored." << endl;
          continue;
        }
        auto fromno = distance(nodes.begin(), find(nodes.begin(), nodes.end(), from));
        if(fromno == numnodes)
          addnode(from);
        auto tono = distance(nodes.begin(), find(nodes.begin(), nodes.end(), to));
        if(tono == numnodes)
          addnode(to);
        edges[fromno].push_back(tono);
      }
    } catch (exception e) {
      break;
    }
  }

  // search cycles
  for(int i = 0; i < numnodes; ++i){
    bitset<maxnumnodes> searched;
    searched.set(i);
    detectcycle(i, i, searched);
  }

  sort(cycles.begin(), cycles.end(), bitset_cmp());

  // search minimal FVSs
  bitset<maxnumnodes> selected, searched;
  search(selected, 0, searched);

  // output
  cout << "#[nodes of minimal FVS] = " << minfvs << endl;
  for(unsigned int i = 0; i < fvs.size(); ++i){
    cout << i+1 << ": ";
    unsigned int tmp = 1;
    for(int j = 0; j < numnodes; ++j)
      if(fvs[i][j]){
        cout << nodes[j];
        if(tmp < minfvs)
          cout << ", ";
        ++tmp;
      }
    cout << endl;
  }

  return 0;
}
