//===-- MCStreamerWrapper.h - Assembly Super Optimiser ---------*- C++ -* -===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// TODO
//
//===----------------------------------------------------------------------===//

#include "llvm/MC/MCStreamer.h"

using namespace llvm;

class MCStreamerWrapper final : public MCStreamer {
public:
  std::vector<MCInst> Insts;

  MCStreamerWrapper(MCContext &Context)
      : MCStreamer(Context) {}

  // We only want to intercept the emission of new instructions.
  void emitInstruction(const MCInst &Inst,
                       const MCSubtargetInfo & /* unused */) override {
    Insts.push_back(Inst);
  }

  bool emitSymbolAttribute(MCSymbol *Symbol, MCSymbolAttr Attribute) override {
    return true;
  }

  void emitCommonSymbol(MCSymbol *Symbol, uint64_t Size,
                        Align ByteAlignment) override {}
  void emitZerofill(MCSection *Section, MCSymbol *Symbol = nullptr,
                    uint64_t Size = 0, Align ByteAlignment = Align(1),
                    SMLoc Loc = SMLoc()) override {}
  void emitGPRel32Value(const MCExpr *Value) override {}
  void beginCOFFSymbolDef(const MCSymbol *Symbol) override {}
  void emitCOFFSymbolStorageClass(int StorageClass) override {}
  void emitCOFFSymbolType(int Type) override {}
  void endCOFFSymbolDef() override {}
};
