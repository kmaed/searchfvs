// searchfvs_withcbc.cc, written by Kazuki Maeda <kmaeda at kmaeda.net>, 2020

#include <coin/CbcModel.hpp>
#include <coin/OsiClpSolverInterface.hpp>

#include "searchfvs.hh"

using namespace std;

// Solve set cover problem by COINOR CBC solver.
void _computeminnumFVS(vector<bitset<maxnumnodes>>& cycles,
                    const int numnodes,
                    unsigned int& minnumFVS,
                    bitset<maxnumnodes>& FVS){
  // construct the coefficient matrix of ILP
  vector<int> rowinds, colinds;
  vector<double> els;
  auto numels = 0;
  int numrows = cycles.size();
  // count nonzero elements
  for(auto i = 0; i < numrows; ++i){
    for(auto j = 0; j < numnodes; ++j)
      if(cycles[i][j]){
        rowinds.push_back(i);
        colinds.push_back(j);
        els.push_back(1);
        ++numels;
      }
  }
  CoinPackedMatrix coefmat(true, &rowinds[0], &colinds[0], &els[0], numels);


  // objective function and constraints
  vector<double> rowlows, rowups;
  for(auto i = 0; i < numrows; ++i){
    // each row constraints >= 1
    rowlows.push_back(1);
    rowups.push_back(COIN_DBL_MAX);
  }
  auto numcols = coefmat.getMajorDim();
  vector<double> objcoefs, collows, colups;
  for(auto j = 0; j < numcols; ++j){
    objcoefs.push_back(1);
    // all variables are in {0, 1}
    collows.push_back(0);
    colups.push_back(1);
  }


  // LP relaxation solver
  OsiClpSolverInterface solver;
  solver.setLogLevel(0);        // no log message
  solver.loadProblem(coefmat, &collows[0], &colups[0], &objcoefs[0], &rowlows[0], &rowups[0]);
  // set all variables as interger
  for(auto j = 0; j < numcols; ++j)
    solver.setInteger(j);

  solver.initialSolve();

  // ILP solver
  CbcModel model(solver);
  model.setLogLevel(0);         // no log message
  model.solver()->setHintParam(OsiDoReducePrint);
  // solve ILP
  model.branchAndBound();

  minnumFVS = model.getObjValue();

  // Is there any way to get all solutions by CBC?
  // The code below will give only one solution.
  const double *solution = model.getColSolution();
  FVS.reset();
  for(auto j = 0; j < numcols; ++j)
    if(solution[j])
      FVS.set(j);
}

void (*computeminnumFVS)(vector<bitset<maxnumnodes>>& cycles,
                         const int numnodes,
                         unsigned int& minnumFVS,
                         bitset<maxnumnodes>& FVS) = _computeminnumFVS;
