// searchfvs.cc, written by Kazuki Maeda <kmaeda at kmaeda.net>, 2017-2018

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <bitset>
#include <algorithm>
#include <errno.h>
#include <getopt.h>

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

unsigned int srtnodeind[maxnumnodes]; // node index sorted by the number appearing in the minimal FVSs

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
                      // all the results of further search will be redundant cycles.
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
  int opt, longindex;
  bool printcycles = false, printhelp = false, nosearch = false;
  bool printstat = false;

  struct option long_options[] =
    {
     {"help", no_argument, NULL, 'h'},
     {"print-cycles", no_argument, NULL, 'c'},
     {"no-search", no_argument, NULL, 'n'},
     {"print-stat", no_argument, NULL, 's'},
     {0, 0, 0, 0}
    };

  while((opt = getopt_long(argc, argv, "chns", long_options, &longindex)) != -1){
    switch(opt){
    case 'h':
      printhelp = true;
      break;
    case 'c':
      printcycles = true;
      break;
    case 'n':
      nosearch = true;
      break;
    case 's':
      printstat = true;
      break;
    }
  }

  if(printhelp || argc - optind < 1){
    cerr << "Usage: " << argv[0] << " [options] <network file>" << endl;
    cerr << "Options:" << endl;
    cerr << "  -h or --help          Print this message and exit." << endl;
    cerr << "  -c or --print-cycles  Print the reduced set of cycles." << endl;
    cerr << "  -n or --no-search     Don't search minimal FVSs." << endl;
    cerr << "  -s or --print-stat    Print statistics of minimal FVSs" << endl;
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
    fin.open(argv[optind]);
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

  sort(cycles.begin(), cycles.end(),
       [](const bitset<maxnumnodes> &x, const bitset<maxnumnodes> &y)->bool{
         return x.count() < y.count();
       });

  // print cycles (if --print-cycles specified)
  if(printcycles){
    cout << "Cycles (reduced):" << endl;
    int n = 1;
    int w = to_string(cycles.size()).length();
    for(auto it = cycles.begin(); it != cycles.end(); ++it){
      bool first = true;
      cout << setw(w) << n << ": ";
      for(int j = 0; j < numnodes; ++j)
        if((*it)[j]){
          if(!first)
            cout << ", ";
          else
            first = false;
          cout << nodes[j];
        }
      cout << endl;
      ++n;
    }
    cout << endl;
  }

  // search minimal FVSs
  if(!nosearch){
    bitset<maxnumnodes> selected, searched;
    search(selected, 0, searched);
    int statfvs[maxnumnodes];
    for(int i = 0; i < maxnumnodes; ++i){
      statfvs[i] = count_if(fvs.begin(), fvs.end(),
                            [&](const bitset<maxnumnodes>& b)->bool{return b[i];});
      srtnodeind[i] = i;
    }

    sort(srtnodeind, srtnodeind+maxnumnodes,
         [&](unsigned int i, unsigned int j)->bool{return statfvs[i] > statfvs[j];});
    sort(fvs.begin(), fvs.end(),
         [](const bitset<maxnumnodes> &x, const bitset<maxnumnodes> &y)->bool{
           for(int i = 0; i < numnodes; ++i){
             if(x[srtnodeind[i]] != y[srtnodeind[i]])
               return x[srtnodeind[i]];
           }
           return true;
         });


    // output
    cout << "#[nodes of minimal FVS] = " << minfvs << endl;
    int w = to_string(fvs.size()).length();
    for(unsigned int i = 0; i < fvs.size(); ++i){
      cout << setw(w) << i+1 << ": ";
      unsigned int tmp = 1;
      for(int j = 0; j < numnodes; ++j)
        if(fvs[i][srtnodeind[j]]){
          cout << nodes[srtnodeind[j]];
          if(tmp < minfvs)
            cout << ", ";
          ++tmp;
        }
      cout << endl;
    }
    cout << endl;

    // print stat. (if --print-stat specified)
    if(printstat){
      cout << "Statistics:" << endl;
      for(int i = 0; i < numnodes; ++i)
        if(statfvs[srtnodeind[i]])
          cout << nodes[srtnodeind[i]] << ":\t" << statfvs[srtnodeind[i]] << endl;
        else
          break;
    }
  }

  return 0;
}
