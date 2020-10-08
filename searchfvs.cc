// searchfvs.cc, written by Kazuki Maeda <kmaeda at kmaeda.net>, 2017-2020

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <errno.h>
#include <getopt.h>

#include "searchfvs.hh"

using namespace std;

int main(int argc, char** argv){
  int opt, longindex;
  bool printcycles = false, printhelp = false, nosearch = false;
  bool nolist = false;
  bool printstat = false, printpolynomial = false, printtree = false;
  int maxtreedepth = digraph::maxnumnodes; // used by outputFVSsastree
  bool onlycomputemin = false;
  vector<string> removenodelist; // List of nodes specified by --remove-node .

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
     {"only-compute-min", no_argument, NULL, 1002},
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
    case 1002:                // --only-compute-min
      if(withcbc)
        onlycomputemin = true;
      else {
        cerr << "Error: This is not \"withcbc\" version; cannot specify --only-compute-min option." << endl;
        printhelp = true;
      }
      break;
    }
  }

  if(printhelp || argc - optind < 1){
    cerr << "Usage: " << argv[0] << " [options] <network file>" << endl;
    cerr << "Options:" << endl;
    cerr << "  -h or --help                Print this message and exit." << endl;
    cerr << "  -n or --no-search           Don't search minimal FVSs (use with --print-cycles)." << endl;
    if(withcbc)
      cerr << "  --only-compute-min          Only compute the mininum number of FVS, print a minimal FVS, and exit." << endl;
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
  digraph dg;

  // read
  auto s = dg.read(argv[optind], removenodelist);
  if(s)
    return s;

  dg.outputremovednodes();
  dg.detectcycles();

  // print cycles (if --print-cycles specified)
  if(printcycles)
    dg.outputcycles(nolist);

  // search minimal FVSs
  if(!nosearch){
    if(withcbc){
      dg.computeminnumFVS(onlycomputemin, nolist);
      if(onlycomputemin)
        return 0;
    }
    dg.dfs();
    dg.calcstatFVS();

    // output
    if(!withcbc)
      dg.outputheader();
    if(!nolist){
      if(printtree){
        dg.outputFVSsastree(printpolynomial, maxtreedepth);
        cout << endl;
      }
      else if(printpolynomial)
        dg.outputFVSsaspolynomial();
      else
        dg.outputFVSs();

      // print stat. (if --print-stat specified)
      if(printstat)
        dg.outputstat();
    }
  }

  return 0;
}
