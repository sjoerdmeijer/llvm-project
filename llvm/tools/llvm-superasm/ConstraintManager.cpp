//===-- ConstraintManager.cpp - Assembly Super Optimiser -------*- C++ -* -===//
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

#define DEBUG_TYPE "llvm-superasm"

#include "ConstraintManager.h"
#include "DepGraph.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

extern cl::opt<bool>  PrintSolverModel;

bool ConstraintManager::createIntConstraints() {
  LLVM_DEBUG(dbgs() << "Creating Z3 constraints:\n\n");
  for (auto *N : DG->Nodes) {
    auto Sym = Solver->mkSymbol(N->getName().c_str(),
                                Solver->getIntegerSort());
    N->setZ3SchedVar(Sym);
    Z3Var2Node.insert({Sym, N});

    Symbols.push_back(Sym);
    auto Zero = Solver->mkInteger(0);
    auto GT = Solver->mkIntGt(Sym, Zero);
    LLVM_DEBUG(dbgs() << "Adding constraint: "; GT->dump(); dbgs() << "\n");
    Solver->addOptConstraint(GT);
  }

  // All variables should get distinct values.
  for (unsigned i = 0; i < DG->Nodes.size(); ++i) {
    for (unsigned j = 0; j < DG->Nodes.size(); ++j) {
      if (i == j)
        continue;

      auto Eq = Solver->mkEqual(Symbols[i], Symbols[j]);
      auto Not = Solver->mkNot(Eq);
      Solver->addOptConstraint(Not);
      LLVM_DEBUG(dbgs() << "Adding constraint: "; Not->dump(); dbgs() << "\n");
    }
  }

  // Add the scheduling constraints.
  for (auto E : DG->Edges) {
    unsigned ID1 = E.Src->getID();
    unsigned ID2 = E.Dst->getID();

    auto Lat = Solver->mkInteger(E.Src->getLatency());
    auto Add = Solver->mkIntBinAdd(Symbols[ID1], Lat);
    auto LT = Solver->mkIntLt(Add, Symbols[ID2]);
    LLVM_DEBUG(dbgs() << "Adding sched constraint: "; LT->dump(); dbgs() << "\n");
    Solver->addOptConstraint(LT);
  }

  if (!DG->Nodes.size())
    return false;
  if (DG->Nodes.size() == 1)
    return Solver->mkMinimizeAndCheck(Symbols[0]);

  auto Add = Solver->mkIntBinAdd(Symbols[0], Symbols[1]);
  for (unsigned i = 2; i < DG->Nodes.size(); ++i)
    Add = Solver->mkIntBinAdd(Add, Symbols[i]);

  LLVM_DEBUG(dbgs() << "Constraint to minimize: "; Add->dump(); dbgs() << "\n");
  return Solver->mkMinimizeAndCheck(Add);
}

void ConstraintManager::setNodeSchedInterpretations() {
  for (auto S : Symbols) {
    APSInt I(32);
    bool Tmp = Solver->getOptInterpretation(S, I);
    (void) Tmp;
    auto Find = Z3Var2Node.find(S);
    Find->second->setZ3SchedVarInterp(I.getExtValue());
  }
}

bool ConstraintManager::solve() {
  if (!createIntConstraints())
    return false;

  setNodeSchedInterpretations();
  return true;
}
