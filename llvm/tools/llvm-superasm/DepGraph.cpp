//===-- DepGraph.cpp - Assembly Super Optimiser ----------------*- C++ -* -===//
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

#include "DepGraph.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSchedule.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/WithColor.h"

bool DepGraph::createNodes(std::vector<MCInst> &Insts) {
  unsigned ID = 0;
  const MCSchedModel &SM = STI->getSchedModel();
  for (auto I : Insts) {
    DGNode N(I, this, ID++);
    if (!N.isSupportedIns(*MRI))
      return false;
    Nodes.push_back(N);

    unsigned SchedClassID = N.getSchedClass();
    const MCSchedClassDesc &SCDesc = *SM.getSchedClassDesc(SchedClassID);
    int Latency = MCSchedModel::computeInstrLatency(*STI, SCDesc);
    LLVM_DEBUG(dbgs() << "Created node " << N.getID() << ", Latency = "
               << Latency << "\n");
    N.setLatency(Latency);
  }
  return true;
}

void DepGraph::createEdges() {
  LLVM_DEBUG(dbgs() << "Analysing " << Nodes.size() << " nodes.\n");
  for (unsigned i = 0; i < Nodes.size(); i++) {
    LLVM_DEBUG(dbgs() << "\n== Analysing instruction: "; Nodes[i].dumpMCInstNL());
    auto I = Nodes[i].Ins;
    bool FoundOutputDep = false;
    for (unsigned j=i+1; j < Nodes.size() ; j++) {
      LLVM_DEBUG(dbgs() << "Looking at: "; Nodes[j].dumpMCInstNL());
      auto J = Nodes[j].Ins;
      for (unsigned IOpIdx = 0; IOpIdx < I.getNumOperands(); IOpIdx++) {
        if (!Nodes[i].isValidReg(IOpIdx))
          continue;
        for (unsigned JOpIdx = 0; JOpIdx < J.getNumOperands(); JOpIdx++) {
          if (!Nodes[j].isValidReg(JOpIdx))
            continue;
          LLVM_DEBUG(dbgs() << "Comparing: ";
                     Nodes[i].dumpOperandInfo(IOpIdx);
                     dbgs() << " == "; Nodes[j].dumpOperandInfo(JOpIdx);
                     dbgs() << "\n");
          if (I.getOperand(IOpIdx).getReg() != J.getOperand(JOpIdx).getReg())
            continue;
          int IWrite = Nodes[i].isWriteReg(IOpIdx);
          int JWrite = Nodes[j].isWriteReg(JOpIdx);

          if (IWrite && JWrite) {
            LLVM_DEBUG(dbgs() << "Found an output dependency in: ";
                       Nodes[j].dumpMCInstNL());
            Edges.push_back(DGEdge(&Nodes[i], &Nodes[j], EdgeType::OutputDep,
                                   IOpIdx));
            FoundOutputDep = true;
          } else if (IWrite && !JWrite) {
            LLVM_DEBUG(dbgs() << "Found a true dependency in: ";
                       Nodes[j].dumpMCInstNL());
            Edges.push_back(DGEdge(&Nodes[i], &Nodes[j], EdgeType::TrueDep,
                                   IOpIdx));
          } else if (!IWrite && JWrite) {
            LLVM_DEBUG(dbgs() << "Found an anti dependency in: ";
                       Nodes[j].dumpMCInstNL());
            Edges.push_back(DGEdge(&Nodes[i], &Nodes[j], EdgeType::AntiDep,
                                   IOpIdx));
            FoundOutputDep = true;
          }
        }
      }
      if (FoundOutputDep)
        break;
    }
  }
}

void DGNode::dumpMCInst() {
  DG->IP->printInst(&Ins, 0, "", *DG->STI, dbgs());
}
void DGNode::dumpMCInstNL() {
  DG->IP->printInst(&Ins, 0, "", *DG->STI, dbgs());
  dbgs() << "\n";
}

void DGNode::dumpOperandInfo(int OpIdx) {
    dbgs() <<
      DG->MRI->getName(Ins.getOperand(OpIdx).getReg());
}

void DGNode::dumpOperandInfo() {
  dbgs() << "Number of operands: " << MCDesc.getNumOperands() << "\n";
  dbgs() << "Number of defs: " << MCDesc.getNumDefs() << "\n";
  dbgs() << "Def reg name: ";
  if (!getNumDefs())
    dbgs() << "<none>\n";
  else
    dbgs() <<
      DG->MRI->getName(Ins.getOperand(getPosDef()).getReg()) <<  "\n";
}

DGNode::DGNode(MCInst I, DepGraph *D, unsigned Num) {
  Ins = I;
  DG = D;
  MCDesc = DG->MCII->get(I.getOpcode());
  ID = Num;
}

bool DGNode::isSupportedIns(const MCRegisterInfo &MRI) {
  if (MCDesc.mayAffectControlFlow(Ins, MRI)) {
    WithColor::error() << "branch instructions are not yet supported:\n";
    dumpMCInstNL();
    return false;
  }
  if (MCDesc.isBarrier()) {
    WithColor::error() << "function calls are not yet supported:\n";
    dumpMCInstNL();
    return false;
  }
  return true;
}
