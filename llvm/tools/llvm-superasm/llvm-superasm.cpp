//===-- llvm-superasm.cpp - Assembly Super Optimiser -----------*- C++ -* -===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//   llvm-superasm [options] <file-name>
//      -march <type>
//      -mcpu <cpu>
//      -o <file>
//
// The target defaults to the host target.
// The cpu defaults to the 'native' host cpu.
// The output defaults to standard output.
//
//===----------------------------------------------------------------------===//

#include "ConstraintManager.h"
#include "DepGraph.h"
#include "MCStreamerWrapper.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInstrAnalysis.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCTargetOptionsCommandFlags.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/MC/MCParser/MCAsmParser.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/WithColor.h"
#include "llvm/TargetParser/Host.h"

using namespace llvm;

static mc::RegisterMCTargetOptionsFlags MOF;

static cl::OptionCategory ToolOptions("Tool Options");
static cl::OptionCategory ViewOptions("View Options");
static cl::OptionCategory MCCategory("MC Options");

static cl::opt<std::string> InputFilename(cl::Positional,
                                          cl::desc("<input file>"),
                                          cl::cat(ToolOptions), cl::init("-"));

static cl::opt<std::string> OutputFilename("o", cl::desc("Output filename"),
                                           cl::init("-"), cl::cat(ToolOptions),
                                           cl::value_desc("filename"));

static cl::opt<std::string>
    ArchName("march",
             cl::desc("Target architecture. "
                      "See -version for available targets"),
             cl::cat(ToolOptions));

static cl::opt<std::string>
    TripleName("mtriple",
               cl::desc("Target triple. See -version for available targets"),
               cl::cat(ToolOptions));

static cl::opt<std::string>
    MCPU("mcpu",
         cl::desc("Target a specific cpu type (-mcpu=help for details)"),
         cl::value_desc("cpu-name"), cl::cat(ToolOptions), cl::init("native"));

static cl::list<std::string>
    MATTRS("mattr", cl::CommaSeparated,
           cl::desc("Target specific attributes (-mattr=help for details)"),
           cl::value_desc("a1,+a2,-a3,..."), cl::cat(ToolOptions));

static cl::opt<int>
    OutputAsmVariant("output-asm-variant",
                     cl::desc("Syntax variant to use for output printing"),
                     cl::cat(ToolOptions), cl::init(-1));

static cl::opt<bool>
    PrintDepGraph("print-dep-graph", cl::cat(ViewOptions), cl::init(false),
                  cl::desc("Print the dependency graph in Dot format"));

static cl::opt<bool>
    PrintSMTConstraints("print-smt-constraints", cl::cat(ViewOptions),
                        cl::init(false),
                        cl::desc("Print the scheduling constraints"));
cl::opt<bool>
    PrintSolverModel("print-solver-model", cl::cat(ViewOptions),
                        cl::init(false),
                        cl::desc("Print the output of the SMT solver"));

static cl::opt<bool>
    ShowInstOperands("show-inst-operands",
                     cl::desc("Show instructions operands as parsed"),
                     cl::cat(MCCategory));

namespace {

const Target *getTarget(const char *ProgName) {
  if (TripleName.empty())
    TripleName = Triple::normalize(sys::getDefaultTargetTriple());
  Triple TheTriple(TripleName);

  // Get the target specific parser.
  std::string Error;
  const Target *TheTarget =
      TargetRegistry::lookupTarget(ArchName, TheTriple, Error);
  if (!TheTarget) {
    errs() << ProgName << ": " << Error;
    return nullptr;
  }

  // Update TripleName with the updated triple from the target lookup.
  TripleName = TheTriple.str();

  // Return the found target.
  return TheTarget;
}

} // end of anonymous namespace

static int ParseInput(const char *ProgName, const Target *TheTarget,
                      SourceMgr &SrcMgr, MCContext &Ctx, MCStreamerWrapper &Str,
                      MCAsmInfo &MAI, MCSubtargetInfo &STI,
                      MCInstrInfo &MCII, MCTargetOptions const &MCOptions) {

  std::unique_ptr<MCAsmParser> Parser(createMCAsmParser(SrcMgr, Ctx, Str, MAI));
  std::unique_ptr<MCTargetAsmParser> TAP(
      TheTarget->createMCAsmParser(STI, *Parser, MCII, MCOptions));

  if (!TAP) {
    WithColor::error(errs(), ProgName)
        << "this target does not support assembly parsing.\n";
    return 1;
  }

  Parser->setShowParsedOperands(ShowInstOperands);
  Parser->setTargetParser(*TAP);
  int Res = Parser->Run(false/*NoInitialTextSection*/);
  return Res;
}

static std::unique_ptr<ToolOutputFile> GetOutputStream(StringRef Path,
    sys::fs::OpenFlags Flags) {
  std::error_code EC;
  auto Out = std::make_unique<ToolOutputFile>(Path, EC, Flags);
  if (EC) {
    WithColor::error() << EC.message() << '\n';
    return nullptr;
  }

  return Out;
}

int main(int argc, char **argv) {
  InitLLVM X(argc, argv);

  // Initialize targets and assembly parsers.
  InitializeAllTargetInfos();
  InitializeAllTargetMCs();
  InitializeAllAsmParsers();

  // Register the Target and CPU printer for --version.
  cl::AddExtraVersionPrinter(sys::printDefaultTargetAndDetectedCPU);

  // Enable printing of available targets when flag --version is specified.
  cl::AddExtraVersionPrinter(TargetRegistry::printRegisteredTargetsForVersion);

  cl::HideUnrelatedOptions({&ToolOptions, &ViewOptions, &MCCategory});

  // Parse flags and initialize target options.
  cl::ParseCommandLineOptions(argc, argv,
                              "llvm machine code performance analyzer.\n");

  // Get the target from the triple. If a triple is not specified, then select
  // the default triple for the host. If the triple doesn't correspond to any
  // registered target, then exit with an error message.
  const char *ProgName = argv[0];
  const Target *TheTarget = getTarget(ProgName);
  if (!TheTarget)
    return 1;

  // GetTarget() may replaced TripleName with a default triple.
  // For safety, reconstruct the Triple object.
  Triple TheTriple(TripleName);

  ErrorOr<std::unique_ptr<MemoryBuffer>> BufferPtr =
      MemoryBuffer::getFileOrSTDIN(InputFilename);
  if (std::error_code EC = BufferPtr.getError()) {
    WithColor::error() << InputFilename << ": " << EC.message() << '\n';
    return 1;
  }

  if (MCPU == "native")
    MCPU = std::string(llvm::sys::getHostCPUName());

  // Package up features to be passed to target/subtarget
  std::string FeaturesStr;
  if (MATTRS.size()) {
    SubtargetFeatures Features;
    for (std::string &MAttr : MATTRS)
      Features.AddFeature(MAttr);
    FeaturesStr = Features.getString();
  }

  std::unique_ptr<MCSubtargetInfo> STI(
      TheTarget->createMCSubtargetInfo(TripleName, MCPU, FeaturesStr));
  assert(STI && "Unable to create subtarget info!");
  if (!STI->isCPUStringValid(MCPU))
    return 1;

  if (!STI->getSchedModel().hasInstrSchedModel()) {
    WithColor::error()
        << "unable to find instruction-level scheduling information for"
        << " target triple '" << TheTriple.normalize() << "' and cpu '" << MCPU
        << "'.\n";

    if (STI->getSchedModel().InstrItineraries)
      WithColor::note()
          << "cpu '" << MCPU << "' provides itineraries. However, "
          << "instruction itineraries are currently unsupported.\n";
    return 1;
  }

  std::unique_ptr<MCRegisterInfo> MRI(TheTarget->createMCRegInfo(TripleName));
  assert(MRI && "Unable to create target register info!");

  MCTargetOptions MCOptions = mc::InitMCTargetOptionsFromFlags();
  std::unique_ptr<MCAsmInfo> MAI(
      TheTarget->createMCAsmInfo(*MRI, TripleName, MCOptions));
  assert(MAI && "Unable to create target asm info!");

  SourceMgr SrcMgr;

  // Tell SrcMgr about this buffer, which is what the parser will pick up.
  SrcMgr.AddNewSourceBuffer(std::move(*BufferPtr), SMLoc());

  MCContext Ctx(TheTriple, MAI.get(), MRI.get(), STI.get(), &SrcMgr);
  std::unique_ptr<MCObjectFileInfo> MOFI(
      TheTarget->createMCObjectFileInfo(Ctx, /*PIC=*/false));
  Ctx.setObjectFileInfo(MOFI.get());

  std::unique_ptr<MCInstrInfo> MCII(TheTarget->createMCInstrInfo());
  assert(MCII && "Unable to create instruction info!");

  std::unique_ptr<MCInstrAnalysis> MCIA(
      TheTarget->createMCInstrAnalysis(MCII.get()));

  // Need to initialize an MCInstPrinter as it is
  // required for initializing the MCTargetStreamer
  // which needs to happen within the CRG.parseAnalysisRegions() call below.
  // Without an MCTargetStreamer, certain assembly directives can trigger a
  // segfault. (For example, the .cv_fpo_proc directive on x86 will segfault if
  // we don't initialize the MCTargetStreamer.)
  unsigned IPtempOutputAsmVariant =
      OutputAsmVariant == -1 ? 0 : OutputAsmVariant;

  MCInstPrinter *IP = TheTarget->createMCInstPrinter(
      Triple(TripleName), IPtempOutputAsmVariant, *MAI, *MCII, *MRI);

  if (!IP) {
    WithColor::error()
        << "unable to create instruction printer for target triple '"
        << TheTriple.normalize() << "' with assembly variant "
        << IPtempOutputAsmVariant << ".\n";
    return 1;
  }

  sys::fs::OpenFlags Flags = sys::fs::OF_TextWithCRLF;
  std::unique_ptr<ToolOutputFile> Out = GetOutputStream(OutputFilename, Flags);
  if (!Out)
    return 1;

  std::unique_ptr<buffer_ostream> BOS;
  raw_pwrite_stream *OS = &Out->os();
  MCStreamerWrapper Str(Ctx);

   // Set up the AsmStreamer.
  std::unique_ptr<MCCodeEmitter> CE;

  std::unique_ptr<MCAsmBackend> MAB(
      TheTarget->createMCAsmBackend(*STI, *MRI, MCOptions));
  auto FOut = std::make_unique<formatted_raw_ostream>(*OS);

  formatted_raw_ostream FOSRef(*OS);
  TheTarget->createAsmTargetStreamer(Str, FOSRef, IP,
                                    /*IsVerboseAsm=*/true);

  DepGraph DG(FOSRef, STI, MCII, IP, MRI);
  ParseInput(ProgName, TheTarget, SrcMgr, Ctx, Str, *MAI, *STI, *MCII, MCOptions);

  if (!DG.createNodes(Str.Insts)) {
    WithColor::error() << "cannot optimise this input.\n";
    return 1;
  }

  DG.createEdges();
  if (PrintDepGraph)
    DG.dumpDotty();
  if (PrintSMTConstraints)
    DG.dumpSMTConstraints();

  ConstraintManager CM(&DG);
  if (!CM.solve())
    return 1;

  if (PrintSolverModel)
    CM.dumpModel();

  DG.scheduleNodes();
  DG.printNodes();
  return 0;
}
