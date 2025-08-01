//===- MCObjectStreamer.h - MCStreamer Object File Interface ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_MC_MCOBJECTSTREAMER_H
#define LLVM_MC_MCOBJECTSTREAMER_H

#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCSection.h"
#include "llvm/MC/MCStreamer.h"

namespace llvm {
class MCContext;
class MCInst;
class MCObjectWriter;
class MCSymbol;
struct MCDwarfFrameInfo;
class MCAssembler;
class MCCodeEmitter;
class MCSubtargetInfo;
class MCExpr;
class MCAsmBackend;
class raw_ostream;
class raw_pwrite_stream;

/// Streaming object file generation interface.
///
/// This class provides an implementation of the MCStreamer interface which is
/// suitable for use with the assembler backend. Specific object file formats
/// are expected to subclass this interface to implement directives specific
/// to that file format or custom semantics expected by the object writer
/// implementation.
class MCObjectStreamer : public MCStreamer {
  std::unique_ptr<MCAssembler> Assembler;
  bool EmitEHFrame;
  bool EmitDebugFrame;

  struct PendingAssignment {
    MCSymbol *Symbol;
    const MCExpr *Value;
  };

  /// A list of conditional assignments we may need to emit if the target
  /// symbol is later emitted.
  DenseMap<const MCSymbol *, SmallVector<PendingAssignment, 1>>
      pendingAssignments;

  void emitInstToData(const MCInst &Inst, const MCSubtargetInfo &);
  void emitCFIStartProcImpl(MCDwarfFrameInfo &Frame) override;
  void emitCFIEndProcImpl(MCDwarfFrameInfo &Frame) override;

protected:
  MCObjectStreamer(MCContext &Context, std::unique_ptr<MCAsmBackend> TAB,
                   std::unique_ptr<MCObjectWriter> OW,
                   std::unique_ptr<MCCodeEmitter> Emitter);
  ~MCObjectStreamer();

public:
  /// state management
  void reset() override;

  /// Object streamers require the integrated assembler.
  bool isIntegratedAssemblerRequired() const override { return true; }

  void emitFrames(MCAsmBackend *MAB);
  MCSymbol *emitCFILabel() override;
  void emitCFISections(bool EH, bool Debug) override;

public:
  void visitUsedSymbol(const MCSymbol &Sym) override;

  MCAssembler &getAssembler() { return *Assembler; }
  MCAssembler *getAssemblerPtr() override;
  /// \name MCStreamer Interface
  /// @{

  // Add a fragment with a variable-size tail and start a new empty fragment.
  void insert(MCFragment *F);

  // Add a new fragment to the current section without a variable-size tail.
  void newFragment();

  void appendContents(size_t Num, char Elt);
  void addFixup(const MCExpr *Value, MCFixupKind Kind);

  void emitLabel(MCSymbol *Symbol, SMLoc Loc = SMLoc()) override;
  virtual void emitLabelAtPos(MCSymbol *Symbol, SMLoc Loc, MCFragment &F,
                              uint64_t Offset);
  void emitAssignment(MCSymbol *Symbol, const MCExpr *Value) override;
  void emitConditionalAssignment(MCSymbol *Symbol,
                                 const MCExpr *Value) override;
  void emitValueImpl(const MCExpr *Value, unsigned Size,
                     SMLoc Loc = SMLoc()) override;
  void emitULEB128Value(const MCExpr *Value) override;
  void emitSLEB128Value(const MCExpr *Value) override;
  void emitWeakReference(MCSymbol *Alias, const MCSymbol *Target) override;
  void changeSection(MCSection *Section, uint32_t Subsection = 0) override;
  void switchSectionNoPrint(MCSection *Section) override;
  void emitInstruction(const MCInst &Inst, const MCSubtargetInfo &STI) override;

  /// Emit an instruction to a special fragment, because this instruction
  /// can change its size during relaxation.
  void emitInstToFragment(const MCInst &Inst, const MCSubtargetInfo &);

  void emitBytes(StringRef Data) override;
  void emitValueToAlignment(Align Alignment, int64_t Fill = 0,
                            uint8_t FillLen = 1,
                            unsigned MaxBytesToEmit = 0) override;
  void emitCodeAlignment(Align ByteAlignment, const MCSubtargetInfo *STI,
                         unsigned MaxBytesToEmit = 0) override;
  void emitValueToOffset(const MCExpr *Offset, unsigned char Value,
                         SMLoc Loc) override;
  void emitDwarfLocDirective(unsigned FileNo, unsigned Line, unsigned Column,
                             unsigned Flags, unsigned Isa,
                             unsigned Discriminator, StringRef FileName,
                             StringRef Comment = {}) override;
  void emitDwarfAdvanceLineAddr(int64_t LineDelta, const MCSymbol *LastLabel,
                                const MCSymbol *Label,
                                unsigned PointerSize) override;
  void emitDwarfLineEndEntry(MCSection *Section, MCSymbol *LastLabel,
                             MCSymbol *EndLabel = nullptr) override;
  void emitDwarfAdvanceFrameAddr(const MCSymbol *LastLabel,
                                 const MCSymbol *Label, SMLoc Loc);
  void emitCVLocDirective(unsigned FunctionId, unsigned FileNo, unsigned Line,
                          unsigned Column, bool PrologueEnd, bool IsStmt,
                          StringRef FileName, SMLoc Loc) override;
  void emitCVLinetableDirective(unsigned FunctionId, const MCSymbol *Begin,
                                const MCSymbol *End) override;
  void emitCVInlineLinetableDirective(unsigned PrimaryFunctionId,
                                      unsigned SourceFileId,
                                      unsigned SourceLineNum,
                                      const MCSymbol *FnStartSym,
                                      const MCSymbol *FnEndSym) override;
  void emitCVDefRangeDirective(
      ArrayRef<std::pair<const MCSymbol *, const MCSymbol *>> Ranges,
      StringRef FixedSizePortion) override;
  void emitCVStringTableDirective() override;
  void emitCVFileChecksumsDirective() override;
  void emitCVFileChecksumOffsetDirective(unsigned FileNo) override;
  void emitRelocDirective(const MCExpr &Offset, StringRef Name,
                          const MCExpr *Expr, SMLoc Loc = {}) override;
  using MCStreamer::emitFill;
  void emitFill(const MCExpr &NumBytes, uint64_t FillValue,
                SMLoc Loc = SMLoc()) override;
  void emitFill(const MCExpr &NumValues, int64_t Size, int64_t Expr,
                SMLoc Loc = SMLoc()) override;
  void emitNops(int64_t NumBytes, int64_t ControlledNopLength, SMLoc Loc,
                const MCSubtargetInfo &STI) override;
  void emitFileDirective(StringRef Filename) override;
  void emitFileDirective(StringRef Filename, StringRef CompilerVersion,
                         StringRef TimeStamp, StringRef Description) override;

  void emitAddrsig() override;
  void emitAddrsigSym(const MCSymbol *Sym) override;

  void finishImpl() override;

  /// Emit the absolute difference between two symbols if possible.
  ///
  /// Emit the absolute difference between \c Hi and \c Lo, as long as we can
  /// compute it.  Currently, that requires that both symbols are in the same
  /// data fragment and that the target has not specified that diff expressions
  /// require relocations to be emitted. Otherwise, do nothing and return
  /// \c false.
  ///
  /// \pre Offset of \c Hi is greater than the offset \c Lo.
  void emitAbsoluteSymbolDiff(const MCSymbol *Hi, const MCSymbol *Lo,
                              unsigned Size) override;

  void emitAbsoluteSymbolDiffAsULEB128(const MCSymbol *Hi,
                                       const MCSymbol *Lo) override;

  bool mayHaveInstructions(MCSection &Sec) const override;

  /// Emits pending conditional assignments that depend on \p Symbol
  /// being emitted.
  void emitPendingAssignments(MCSymbol *Symbol);
};

} // end namespace llvm

#endif
