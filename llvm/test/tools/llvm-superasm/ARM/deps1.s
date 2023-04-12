# RUN: llvm-superasm -mtriple=thumbv8.1-m.main-none-none-eabi -mcpu=cortex-m55 -print-dep-graph < %s 2>&1 | FileCheck %s

# CHECK:       digraph deps {
# CHECK-NEXT:    N0 [label = "   mul     r3, r1, r2"];
# CHECK-NEXT:    N1 [label = "   add.w   r2, r5, r6"];
# CHECK-NEXT:    N0 -> N1 [label = "R2:anti"];
# CHECK-NEXT:  }

mul r3,r1,r2
add r2,r5,r6
