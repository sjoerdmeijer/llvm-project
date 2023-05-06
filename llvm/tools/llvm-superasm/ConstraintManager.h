//===-- ConstraintManager.h - Assembly Super Optimiser ---------*- C++ -* -===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  TODO
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TOOLS_SUPERASM_CONSTRAINT_MANAGER
#define  LLVM_TOOLS_SUPERASM_CONSTRAINT_MANAGER

#include "llvm/Support/Debug.h"
#include "llvm/Support/SMTAPI.h"
#include <map>

using namespace llvm;

class DepGraph;
class DGNode;

class ConstraintManager {
  DepGraph *DG;
  std::vector<SMTExprRef> Symbols;

  mutable llvm::SMTSolverRef Solver = llvm::CreateZ3Solver();
  std::map<SMTExprRef, DGNode*> Z3Var2Node;
public:

  ConstraintManager(DepGraph *DG) : DG(DG) { }
  bool solve();
  bool createIntConstraints();
  void setNodeSchedInterpretations();
  void dumpModel() { dbgs() << "\nSolver model:\n\n"; Solver->dumpOpt(); }
};

#endif
