//===-- DepGraph.h - Assembly Super Optimiser ------------------*- C++ -* -===//
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

#ifndef LLVM_TOOLS_SUPER_ASM_H
#define LLVM_TOOLS_SUPER_ASM_H

#include <vector>
#include "ConstraintManager.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;

class DepGraph;

class DGNode {
  MCInstrDesc MCDesc;
  unsigned ID;
  int Latency;

  SMTExprRef Z3SchedVar;
  int64_t Z3SchedVarInterpretation;

public:
  DepGraph *DG;
  MCInst Ins;

  DGNode(MCInst I, DepGraph *DG, unsigned ID);

  unsigned getNumDefs() { return MCDesc.getNumDefs(); }
  unsigned getPosDef() { return getNumDefs() - 1; }
  std::string getName() { return "N" + std::to_string(ID); }
  bool isSupportedIns(const MCRegisterInfo &MRI);
  void setLatency(int L) { Latency = L; }
  int getLatency() { return Latency; }
  unsigned getID() { return ID; }
  unsigned getSchedClass() { return MCDesc.getSchedClass(); }
  void setZ3SchedVar(SMTExprRef V) { Z3SchedVar = V ; }
  void setZ3SchedVarInterp(unsigned V) { Z3SchedVarInterpretation = V ; }
  unsigned getZ3SchedVarInterp() { return Z3SchedVarInterpretation; }

  bool isValidReg(int OpIdx) {
    return Ins.getOperand(OpIdx).isValid() && Ins.getOperand(OpIdx).isReg()
      && Ins.getOperand(OpIdx).getReg();
  }

  bool isWriteReg(unsigned OpIdx) {
    if (!getNumDefs())
      return false;
    return OpIdx < getNumDefs();
  }

  void dumpMCInst();
  void dumpMCInstNL();
  void dumpOperandInfo();
  void dumpOperandInfo(int OpIdx);

  void dumpDotty() {
    dbgs() << getName() << " [label = \"";
    dumpMCInst();
    dbgs() << "\"];\n";
  }
};

enum class EdgeType { AntiDep, TrueDep, OutputDep };

class DGEdge {
public:
  DGNode *Src;
  DGNode *Dst;
  EdgeType DepType;
  unsigned SrcOpIdx;

  DGEdge(DGNode *S, DGNode *D, EdgeType E, unsigned I)
    : Src(S), Dst(D), DepType(E), SrcOpIdx(I) {}

  void dumpDotty() {
    dbgs() << Src->getName();
    dbgs() << " -> ";
    dbgs() << Dst->getName();
    dbgs() << " [label = \"";
    Src->dumpOperandInfo(SrcOpIdx);
    dbgs() << ":";
    if (DepType == EdgeType::AntiDep)
      dbgs() << "anti";
    else if (DepType == EdgeType::TrueDep)
      dbgs() << "true";
    else
      dbgs() << "output";
    dbgs() << "\"];\n";

  }
};

class DepGraph {
  formatted_raw_ostream &FOSRef;

public:
  std::vector<DGNode*> Nodes;
  std::vector<DGEdge> Edges;

  std::unique_ptr<MCInstrInfo> &MCII;
  std::unique_ptr<MCSubtargetInfo> &STI;
  MCInstPrinter *IP;
  std::unique_ptr<MCRegisterInfo> &MRI;

  DepGraph(formatted_raw_ostream &FOSRef,
           std::unique_ptr<MCSubtargetInfo> &STI,
           std::unique_ptr<MCInstrInfo> &MCII, MCInstPrinter *IP,
           std::unique_ptr<MCRegisterInfo> &MRI) :
      FOSRef(FOSRef), MCII(MCII), STI(STI), IP(IP), MRI(MRI) {}

  void createEdges();
  bool createNodes(std::vector<MCInst> &Insts);

  void dumpDotty() {
    dbgs() << "\ndigraph deps {\n";
    for (auto *N : Nodes)
      N->dumpDotty();
    for (auto E : Edges)
      E.dumpDotty();
    dbgs() << "}\n";
  }

  void dumpSMTConstraints();
  void scheduleNodes();
  void printNodes();

  void printNodeAddrs() {
    for (auto *N : Nodes) {
      dbgs() << "Node: " << N << "\n";
    }
  }
};

#endif
