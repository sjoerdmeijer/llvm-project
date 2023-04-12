# RUN: llvm-superasm -mtriple=thumbv8.1-m.main-none-none-eabi -mcpu=cortex-m55 -print-dep-graph < %s 2>&1 | FileCheck %s

# CHECK: digraph deps {
# CHECK-NEXT:  N0 [label = "   vstrw.32        q2, [r0, #32]"];
# CHECK-NEXT:  N1 [label = "   vstrw.32        q0, [r0], #48"];
# CHECK-NEXT:  N0 -> N1 [label = "R0:anti"];
# CHECK-NEXT:  }

vstrw.32        q2, [r0, #32]
vstrw.32        q0, [r0], #48
