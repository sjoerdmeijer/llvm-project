include_directories(include)

set(LLVM_LINK_COMPONENTS
  AllTargetsAsmParsers
  AllTargetsDescs
  AllTargetsDisassemblers
  AllTargetsInfos
  MCA
  MC
  MCParser
  Support
  TargetParser
  )

add_llvm_tool(llvm-superasm
  llvm-superasm.cpp
  )

set(LLVM_SUPERASM_SOURCE_DIR ${CURRENT_SOURCE_DIR})
