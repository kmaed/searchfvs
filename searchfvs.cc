// searchfvs.cc, written by Kazuki Maeda <kmaeda at kmaeda.net>, 2017-2020

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

const int maxnumnodes = 10000;   // The max number of nodes of input network.
                                 // If the input network has more nodes, increase this value
                                 // and recompile.
                                 // Developer notes: we can use boost::dynamic_bitset
                                 // as an alternative of std::bitset. However, dynamic_bitset
                                 // is much slower than bitset.

vector<string> nodes;
vector<string> removenodelist; // List of nodes specified by --remove-node .
vector<string> removednodes;   // Actually removed nodes.

vector<vector<int>> edges;      // edges[i] = {j0, j1, j2, ...}: i -> j

void addandreducecycles(bitset<maxnumnodes>& cycle,
                        vector<bitset<maxnumnodes>>& cycles){
  // add?
  for(const auto& c: cycles)
    if((cycle | c) == cycle)
      return;

  // reduce (leave only local minimum cycles)
  for(auto it = cycles.begin(); it != cycles.end(); ){
    if((cycle | *it) == *it)
      it = cycles.erase(it);
    else
      ++it;
  }
  cycles.push_back(cycle);
}

void detectcycles(int start, int i,
                  bitset<maxnumnodes>& searched, bitset<maxnumnodes>& path,
                  vector<bitset<maxnumnodes>>& cycles){
  path.set(i);
  bitset<maxnumnodes> nextsearched = searched;
  for(const auto& v: edges[i])
    nextsearched.set(v);        // prevent redundant search
                                // E.g., if you have already found the path 0 -> 1,
                                // then you can omit the path 0 -> 2 -> 1.
  bool endflag = false;
  bool cycleflag = false;
  for(const auto& v: edges[i]){
    if(v == start){
      cycleflag = true;
      break;
    } else if(path[v]){
      endflag = true; // If a branch returns to the node already passed,
                      // all the results of further search will be redundant cycles.
      break;
    }
  }

  if(cycleflag){
    if(path.count())
      addandreducecycles(path, cycles);
  } else if(!endflag) {
    for(const auto& v: edges[i]){
      if(v < start || searched[v])
        continue;
      else
        detectcycles(start, v, nextsearched, path, cycles);
    }
  }
  path.reset(i);
}

void search(vector<bitset<maxnumnodes>>& cycles,
            bitset<maxnumnodes>& selected,
            unsigned int cyclenum,
            bitset<maxnumnodes>& searched,
            const int numnodes,
            vector<bitset<maxnumnodes>>& FVSs,
            unsigned int& minnumFVS) {
  bitset<maxnumnodes> nextsearched = searched;
  while(cyclenum < cycles.size()){
    if(!(selected & cycles[cyclenum]).count()){
      if(selected.count() == minnumFVS)
        return;
      for(int i = 0; i < numnodes; ++i)
        if(cycles[cyclenum][i] && !nextsearched[i]){
          selected.set(i);
          nextsearched.set(i);   // prevent duplicate listing
          search(cycles, selected, cyclenum+1, nextsearched, numnodes, FVSs, minnumFVS);
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

void addnode(string node, int& numnodes){
  nodes.push_back(node);
  edges.push_back(vector<int>());
  ++numnodes;
  if(numnodes > maxnumnodes){
    cerr << "Error: the number of nodes in the input network is too large!" << endl;
    cerr << "       Increase the value of maxnumnodes and recompile the program." << endl;
    exit(1);
  }
}

void outputcycles(bool nolist, const int numnodes, vector<bitset<maxnumnodes>>& cycles){
  cout << "#[chordless cycles] = " << cycles.size() << endl;
  if(!nolist){
    int n = 1;
    int w = to_string(cycles.size()).length();
    for(const auto& c: cycles){
      bool first = true;
      cout << setw(w) << n << ": ";
      for(int j = 0; j < numnodes; ++j)
        if(c[j]){
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
}

void calcstatFVS(vector<bitset<maxnumnodes>> listFVSs, int* statFVS){
  for(int i = 0; i < maxnumnodes; ++i)
    statFVS[i] = count_if(listFVSs.begin(), listFVSs.end(),
                          [&](const bitset<maxnumnodes>& b)->bool{return b[i];});
}

void outputFVSs(const int numnodes,
                const unsigned int* srtnodeind,
                const vector<bitset<maxnumnodes>>& FVSs,
                const unsigned int minnumFVS){
  int w = to_string(FVSs.size()).length();
  for(unsigned int i = 0; i < FVSs.size(); ++i){
    cout << setw(w) << i+1 << ": ";
    unsigned int tmp = 1;
    for(int j = 0; j < numnodes; ++j)
      if(FVSs[i][srtnodeind[j]]){
        cout << nodes[srtnodeind[j]];
        if(tmp < minnumFVS)
          cout << ", ";
        ++tmp;
      }
    cout << endl;
  }
  cout << endl;
}

void outputFVSsaspolynomial(const int numnodes,
                            const unsigned int* srtnodeind,
                            const vector<bitset<maxnumnodes>>& FVSs,
                            const unsigned int minnumFVS){
  for(unsigned int i = 0; i < FVSs.size(); ++i){
    unsigned int tmp = 1;
    for(int j = 0; j < numnodes; ++j)
      if(FVSs[i][srtnodeind[j]]){
        cout << '"' << nodes[srtnodeind[j]] << '"';
        if(tmp < minnumFVS)
          cout << " * ";
        ++tmp;
      }
    cout << endl;
    if(i != FVSs.size()-1)
      cout << " + ";
  }
  cout << endl;
}


void outputFVSsastree(const vector<bitset<maxnumnodes>>& currentFVSs, const int* statFVS,
                      int pnum, int level, bool printpolynomial, const int maxtreedepth){
  static int prevlevel = -1;
  if(!currentFVSs.size())
    return;

  int targetnode = max_element(statFVS, statFVS+maxnumnodes)-statFVS;
  vector<bitset<maxnumnodes>> hFVSs;
  vector<bitset<maxnumnodes>> nFVSs;
  for(const auto& c: currentFVSs){
    if(c[targetnode]){
      bitset<maxnumnodes> tmp = c;
      tmp.reset(targetnode);
      if(tmp.any())
        hFVSs.push_back(tmp);
    } else
      nFVSs.push_back(c);
  }

  if(printpolynomial){
    int difflevel = prevlevel - level;
    if(difflevel >= 0)
      cout << " + ";
    cout << "\"" << nodes[targetnode] << "\"";
    if(hFVSs.size() != 0)
      cout << " * (";
    prevlevel = level;
  } else {
    for(int i = 0; i < level; ++i)
      cout << string("|   ");
    cout << level+1 << ": " << nodes[targetnode] << " (" << statFVS[targetnode] << "/" << pnum << ")" << endl;
  }

  int stathFVSs[maxnumnodes];
  int statnFVSs[maxnumnodes];
  if(hFVSs.size())
    calcstatFVS(hFVSs, stathFVSs);
  if(nFVSs.size())
    calcstatFVS(nFVSs, statnFVSs);

  if(printpolynomial || level+1 < maxtreedepth)
    outputFVSsastree(hFVSs, stathFVSs, hFVSs.size(), level+1, printpolynomial, maxtreedepth);
  if(printpolynomial && hFVSs.size() != 0)
    cout << ")";
  outputFVSsastree(nFVSs, statnFVSs, pnum, level, printpolynomial, maxtreedepth);
}

void outputstat(const int* statFVS,
                const int numnodes,
                const vector<bitset<maxnumnodes>>& FVSs,
                const unsigned int* srtnodeind){
  int ws = max_element(nodes.begin(), nodes.end(),
                      [](string& x, string& y)->bool{return x.length() < y.length();})->length();
  int w = to_string(FVSs.size()).length();
  cout << "Statistics:" << endl;
  for(int i = 0; i < numnodes; ++i)
    if(statFVS[srtnodeind[i]])
      cout << setw(ws) << nodes[srtnodeind[i]] << ": " << setw(w) << statFVS[srtnodeind[i]] << endl;
    else
      break;
  cout << endl;
}

int main(int argc, char** argv){
  int numnodes = 0, numedges = 0;
  unsigned int srtnodeind[maxnumnodes]; // node index sorted by the number appearing in the minimal FVSs



  int opt, longindex;
  bool printcycles = false, printhelp = false, nosearch = false;
  bool nolist = false;
  bool printstat = false, printpolynomial = false, printtree = false;
  int maxtreedepth = maxnumnodes; // used by outputFVSsastree

  struct option long_options[] =
    {
     {"print-cycles", no_argument, NULL, 'c'},
     {"help", no_argument, NULL, 'h'},
     {"no-list", no_argument, NULL, 1001},
     {"no-search", no_argument, NULL, 'n'},
     {"print-polynomial", no_argument, NULL, 'p'},
     {"remove-node", required_argument, NULL, 'r'},
     {"print-stat", no_argument, NULL, 's'},
     {"print-tree", no_argument, NULL, 't'},
     {"max-tree-depth", required_argument, NULL, 1000},
     {0, 0, 0, 0}
    };

  while((opt = getopt_long(argc, argv, "chnpr:st", long_options, &longindex)) != -1){
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
    case 'p':
      printpolynomial = true;
      break;
    case 'r':
      removenodelist.push_back(optarg);
      break;
    case 's':
      printstat = true;
      break;
    case 't':
      printtree = true;
      break;
    case 1000:                  // --max-tree-depth
      maxtreedepth = atoi(optarg);
      break;
    case 1001:                // --no-list
      nolist = true;
      break;
    }
  }

  if(printhelp || argc - optind < 1){
    cerr << "Usage: " << argv[0] << " [options] <network file>" << endl;
    cerr << "Options:" << endl;
    cerr << "  -h or --help                Print this message and exit." << endl;
    cerr << "  -n or --no-search           Don't search minimal FVSs (use with --print-cycles)." << endl;
    cerr << "  --no-list                   Don't show list of chordless cycles and minimal FVSs." << endl;
    cerr << "  -c or --print-cycles        Print the set of chordless cycles at the head." << endl;
    cerr << "  -t or --print-tree          Print the tree list of minimal FVSs." << endl;
    cerr << "  --max-tree-depth <depth>    Restrict tree level to specified level (use with --print-tree)." << endl;
    cerr << "  -p or --print-polynomial    Print the list of minimal FVSs as a polynomial." << endl;
    cerr << "  -s or --print-stat          Print statistics of minimal FVSs." << endl;
    cerr << "  -r or --remove-node <node>  Remove specified node." << endl;
    return 1;
  }

  // init
  nodes = vector<string>();
  edges = vector<vector<int>>();

  // read
  ifstream fin;
  fin.exceptions(ios::failbit);
  try{
    fin.open(argv[optind]);
  } catch (exception& e){
    cerr << argv[0] << ": " << argv[optind] << ": " << strerror(errno) << endl;
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
          cerr << argv[optind] << ":" << lineno << ": warning: illegal input line \"" << tmp << "\", ignored." << endl;
          continue;
        }
        auto fromrem = find(removenodelist.begin(), removenodelist.end(), from);
        auto torem = find(removenodelist.begin(), removenodelist.end(), to);
        if(removenodelist.size() &&
           (fromrem != removenodelist.end() || torem != removenodelist.end())){
          if(fromrem != removenodelist.end() &&
             find(removednodes.begin(), removednodes.end(), from) == removednodes.end())
            removednodes.push_back(from);
          if(torem != removenodelist.end() &&
             find(removednodes.begin(), removednodes.end(), to) == removednodes.end())
            removednodes.push_back(to);
          continue;
        }
        auto fromno = distance(nodes.begin(), find(nodes.begin(), nodes.end(), from));
        if(fromno == numnodes)
          addnode(from, numnodes);
        auto tono = distance(nodes.begin(), find(nodes.begin(), nodes.end(), to));
        if(tono == numnodes)
          addnode(to, numnodes);
        if(find(edges[fromno].begin(), edges[fromno].end(), tono) == edges[fromno].end()){
          ++numedges;
          edges[fromno].push_back(tono);
        }
      }
    } catch (exception& e) {
      break;
    }
  }

  // print removed nodes by --remove-node
  if(!removednodes.empty()){
    cout << "Removed nodes: ";
    bool first = true;
    for(const auto& r: removednodes){
      if(!first)
        cout << ", ";
      else
        first = false;
      cout << r;
    }
    cout << endl << endl;
  }


  // search chordless cycles
  vector<bitset<maxnumnodes>> cycles; // cycles[i][j]: if node j is in cycle i
                                // Note: this is the set of "local minimum" cycles (chordless cycles).
                                // E.g., if there are cycles {1101} and {1001},
                                // only {1001} is included in the cycles variable.
  bitset<maxnumnodes> path;
  for(auto i = 0; i < numnodes; ++i){
    bitset<maxnumnodes> searched;
    searched.set(i);
    detectcycles(i, i, searched, path, cycles);
  }

  sort(cycles.begin(), cycles.end(),
       [](const bitset<maxnumnodes> &x, const bitset<maxnumnodes> &y)->bool{
         return x.count() < y.count();
       });

  // print cycles (if --print-cycles specified)
  if(printcycles)
    outputcycles(nolist, numnodes, cycles);

  // search minimal FVSs
  unsigned int minnumFVS = maxnumnodes;
  vector<bitset<maxnumnodes>> FVSs;
  if(!nosearch){
    bitset<maxnumnodes> selected, searched;
    search(cycles, selected, 0, searched, numnodes, FVSs, minnumFVS);
    int statFVS[maxnumnodes];
    calcstatFVS(FVSs, statFVS);
    for(int i = 0; i < maxnumnodes; ++i)
      srtnodeind[i] = i;

    sort(srtnodeind, srtnodeind+maxnumnodes,
         [&](const unsigned int i, const unsigned int j) {
           return statFVS[i] > statFVS[j];
         });
    sort(FVSs.begin(), FVSs.end(),
         [&](const bitset<maxnumnodes>& x, const bitset<maxnumnodes>& y) -> bool {
           for(int i = 0; i < numnodes; ++i){
             if(x[srtnodeind[i]] != y[srtnodeind[i]])
               return x[srtnodeind[i]];
           }
           return true;
         });

    // output
    cout << "#nodes,#edges,#[nodes of minimal FVS] = " << numnodes << "," << numedges << "," << minnumFVS << endl;
    if(!nolist){
      if(printtree){
        outputFVSsastree(FVSs, statFVS, FVSs.size(), 0, printpolynomial, maxtreedepth);
        cout << endl;
      }
      else if(printpolynomial)
        outputFVSsaspolynomial(numnodes, srtnodeind, FVSs, minnumFVS);
      else
        outputFVSs(numnodes, srtnodeind, FVSs, minnumFVS);

      // print stat. (if --print-stat specified)
      if(printstat)
        outputstat(statFVS, numnodes, FVSs, srtnodeind);
    }
  }

  return 0;
}
