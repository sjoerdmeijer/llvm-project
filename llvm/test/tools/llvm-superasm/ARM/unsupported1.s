# RUN: not llvm-superasm -mtriple=thumbv8.1-m.main-none-none-eabi -mcpu=cortex-m55 -print-dep-graph < %s 2>&1 | FileCheck %s

# CHECK:  error: branch instructions are not yet supported:
# CHECK:          b       LOOP
# CHECK:  error: cannot optimise this input.

LOOP:
  add r1, r2, r3
b LOOP
