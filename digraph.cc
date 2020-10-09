// digraph.cc, written by Kazuki Maeda <kmaeda at kmaeda.net>, 2020

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstring>
#include <algorithm>
#include "digraph.hh"

using namespace std;

digraph::digraph(){
  numnodes = 0;
  numedges = 0;
  minnumFVS = maxnumnodes;
}

int digraph::read(const string filename, const vector<string>& removenodelist){
  ifstream fin;
  fin.exceptions(ios::failbit);
  try{
    fin.open(filename);
  } catch (exception& e){
    cerr << filename << ": " << strerror(errno) << endl;
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
          cerr << filename << ":" << lineno << ": warning: illegal input line \"" << tmp << "\", ignored." << endl;
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
          addnode(from);
        auto tono = distance(nodes.begin(), find(nodes.begin(), nodes.end(), to));
        if(tono == numnodes)
          addnode(to);
        if(find(edges[fromno].begin(), edges[fromno].end(), tono) == edges[fromno].end()){
          ++numedges;
          edges[fromno].push_back(tono);
        }
      }
    } catch (exception& e) {
      break;
    }
  }
  return 0;
}

void digraph::addnode(const string node){
  nodes.push_back(node);
  edges.emplace_back();
  ++numnodes;
  if(numnodes > maxnumnodes){
    cerr << "Error: the number of nodes in the input network is too large!" << endl;
    cerr << "       Increase the value of maxnumnodes and recompile the program." << endl;
    exit(1);
  }
}

void digraph::addandreducecycles(const bitset<maxnumnodes>& cycle){
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

void digraph::_detectcycles(const int start, const int i,
                            const bitset<maxnumnodes>& searched,
                            bitset<maxnumnodes>& path){
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
      addandreducecycles(path);
  } else if(!endflag) {
    for(const auto& v: edges[i]){
      if(v < start || searched[v])
        continue;
      else
        _detectcycles(start, v, nextsearched, path);
    }
  }
  path.reset(i);
}

void digraph::detectcycles(){
  bitset<maxnumnodes> path;
  for(auto i = 0; i < numnodes; ++i){
    bitset<maxnumnodes> searched;
    searched.set(i);
    _detectcycles(i, i, searched, path);
  }
  sort(cycles.begin(), cycles.end(),
       [](const bitset<maxnumnodes> &x, const bitset<maxnumnodes> &y)->bool{
         return x.count() < y.count();
       });
}

// Solve set cover problem by simple DFS.
void digraph::_dfs(unsigned int cyclenum,
                   bitset<maxnumnodes>& selected,
                   bitset<maxnumnodes>& searched) {
  bitset<maxnumnodes> nextsearched = searched;
  while(cyclenum < cycles.size()){
    if(!(selected & cycles[cyclenum]).count()){ // already covered?
      if(selected.count() == minnumFVS)
        return;
      for(int i = 0; i < numnodes; ++i)
        if(cycles[cyclenum][i] && !nextsearched[i]){
          selected.set(i);
          nextsearched.set(i);   // prevent duplicate listing
          _dfs(cyclenum+1, selected, nextsearched);
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

void digraph::outputcycles(const bool nolist) const {
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


void digraph::calcstatFVS(){
  _calcstatFVS(FVSs, statFVS);
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
}

void digraph::outputremovednodes() const {
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
}

void digraph::outputstat() const {
  int ws = max_element(nodes.begin(), nodes.end(),
                      [](const string& x, const string& y)->bool{return x.length() < y.length();})->length();
  int w = to_string(FVSs.size()).length();
  cout << "Statistics:" << endl;
  for(int i = 0; i < numnodes; ++i)
    if(statFVS[srtnodeind[i]])
      cout << setw(ws) << nodes[srtnodeind[i]] << ": " << setw(w) << statFVS[srtnodeind[i]] << endl;
    else
      break;
  cout << endl;
}

void digraph::outputFVSs() const {
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

void digraph::outputFVSsaspolynomial() const {
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


void digraph::_outputFVSsastree(const vector<bitset<maxnumnodes>>& currentFVSs,
                                const int* statFVS,
                                const int pnum,
                                const int level,
                                const bool printpolynomial,
                                const int maxtreedepth) const {
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
    _calcstatFVS(hFVSs, stathFVSs);
  if(nFVSs.size())
    _calcstatFVS(nFVSs, statnFVSs);

  if(printpolynomial || level+1 < maxtreedepth)
    _outputFVSsastree(hFVSs, stathFVSs, hFVSs.size(), level+1, printpolynomial, maxtreedepth);
  if(printpolynomial && hFVSs.size() != 0)
    cout << ")";
  _outputFVSsastree(nFVSs, statnFVSs, pnum, level, printpolynomial, maxtreedepth);
}
