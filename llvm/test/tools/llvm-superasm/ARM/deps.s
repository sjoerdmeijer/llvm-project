# RUN: llvm-superasm -mtriple=thumbv8.1-m.main-none-none-eabi -mcpu=cortex-m55 -print-dep-graph < %s 2>&1 | FileCheck %s

#CHECK:       digraph deps {
#CHECK-NEXT:  N0 [label = "   add.w   r1, r2, r3"];
#CHECK-NEXT:  N1 [label = "   add.w   r4, r2, r1"];
#CHECK-NEXT:  N2 [label = "   add.w   r1, r2, r3"];
#CHECK-NEXT:  N0 -> N1 [label = "R1:true"];
#CHECK-NEXT:  N0 -> N2 [label = "R1:output"];
#CHECK-NEXT:  N1 -> N2 [label = "R1:anti"];
#CHECK-NEXT:  }

add r1, r2, r3
add r4, r2, r1
add r1, r2, r3
