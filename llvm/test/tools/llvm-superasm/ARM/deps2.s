# RUN: llvm-superasm -mtriple=thumbv8.1-m.main-none-none-eabi -mcpu=cortex-m55 -print-dep-graph < %s 2>&1 | FileCheck %s

# CHECK:       digraph deps {
# CHECK-NEXT:  N0 [label = "   vldrw.u32       q0, [r0]"];
# CHECK-NEXT:  N1 [label = "   vldrw.u32       q1, [r0, #16]"];
# CHECK-NEXT:  N2 [label = "   vldrw.u32       q2, [r0, #32]"];
# CHECK-NEXT:  N3 [label = "   vldrw.u32       q7, [r1], #16"];
# CHECK-NEXT:  N4 [label = "   vmulh.u32       q0, q0, q7"];
# CHECK-NEXT:  N5 [label = "   vmulh.u32       q1, q1, q7"];
# CHECK-NEXT:  N6 [label = "   vmulh.u32       q2, q2, q7"];
# CHECK-NEXT:  N7 [label = "   vadd.i32        q0, q0, q0"];
# CHECK-NEXT:  N8 [label = "   vadd.i32        q0, q0, q7"];
# CHECK-NEXT:  N9 [label = "   vadd.i32        q1, q1, q1"];
# CHECK-NEXT:  N10 [label = "  vadd.i32        q1, q1, q7"];
# CHECK-NEXT:  N11 [label = "  vadd.i32        q2, q2, q2"];
# CHECK-NEXT:  N12 [label = "  vadd.i32        q2, q2, q7"];
# CHECK-NEXT:  N13 [label = "  vstrw.32        q1, [r0, #16]"];
# CHECK-NEXT:  N14 [label = "  vstrw.32        q2, [r0, #32]"];
# CHECK-NEXT:  N15 [label = "  vstrw.32        q0, [r0], #48"];
# CHECK-NEXT:  N0 -> N4 [label = "Q0:output"];
# CHECK-NEXT:  N0 -> N4 [label = "Q0:true"];
# CHECK-NEXT:  N1 -> N5 [label = "Q1:output"];
# CHECK-NEXT:  N1 -> N5 [label = "Q1:true"];
# CHECK-NEXT:  N2 -> N6 [label = "Q2:output"];
# CHECK-NEXT:  N2 -> N6 [label = "Q2:true"];
# CHECK-NEXT:  N3 -> N4 [label = "Q7:true"];
# CHECK-NEXT:  N3 -> N5 [label = "Q7:true"];
# CHECK-NEXT:  N3 -> N6 [label = "Q7:true"];
# CHECK-NEXT:  N3 -> N8 [label = "Q7:true"];
# CHECK-NEXT:  N3 -> N10 [label = "Q7:true"];
# CHECK-NEXT:  N3 -> N12 [label = "Q7:true"];
# CHECK-NEXT:  N4 -> N7 [label = "Q0:output"];
# CHECK-NEXT:  N4 -> N7 [label = "Q0:true"];
# CHECK-NEXT:  N4 -> N7 [label = "Q0:true"];
# CHECK-NEXT:  N4 -> N7 [label = "Q0:anti"];
# CHECK-NEXT:  N5 -> N9 [label = "Q1:output"];
# CHECK-NEXT:  N5 -> N9 [label = "Q1:true"];
# CHECK-NEXT:  N5 -> N9 [label = "Q1:true"];
# CHECK-NEXT:  N5 -> N9 [label = "Q1:anti"];
# CHECK-NEXT:  N6 -> N11 [label = "Q2:output"];
# CHECK-NEXT:  N6 -> N11 [label = "Q2:true"];
# CHECK-NEXT:  N6 -> N11 [label = "Q2:true"];
# CHECK-NEXT:  N6 -> N11 [label = "Q2:anti"];
# CHECK-NEXT:  N7 -> N8 [label = "Q0:output"];
# CHECK-NEXT:  N7 -> N8 [label = "Q0:true"];
# CHECK-NEXT:  N7 -> N8 [label = "Q0:anti"];
# CHECK-NEXT:  N7 -> N8 [label = "Q0:anti"];
# CHECK-NEXT:  N8 -> N15 [label = "Q0:true"];
# CHECK-NEXT:  N9 -> N10 [label = "Q1:output"];
# CHECK-NEXT:  N9 -> N10 [label = "Q1:true"];
# CHECK-NEXT:  N9 -> N10 [label = "Q1:anti"];
# CHECK-NEXT:  N9 -> N10 [label = "Q1:anti"];
# CHECK-NEXT:  N10 -> N13 [label = "Q1:true"];
# CHECK-NEXT:  N11 -> N12 [label = "Q2:output"];
# CHECK-NEXT:  N11 -> N12 [label = "Q2:true"];
# CHECK-NEXT:  N11 -> N12 [label = "Q2:anti"];
# CHECK-NEXT:  N11 -> N12 [label = "Q2:anti"];
# CHECK-NEXT:  N12 -> N14 [label = "Q2:true"];
# CHECK-NEXT:  N13 -> N15 [label = "R0:anti"];
# CHECK-NEXT:  N14 -> N15 [label = "R0:anti"];
# CHECK-NEXT:  }

vldrw.u32 q0, [r0]
vldrw.u32 q1, [r0, #16]
vldrw.u32 q2, [r0, #32]
vldrw.u32 q7, [r1], #16
vmulh.u32 q0, q0, q7
vmulh.u32 q1, q1, q7
vmulh.u32 q2, q2, q7
vadd.u32 q0, q0, q0
vadd.u32 q0, q0, q7
vadd.u32 q1, q1, q1
vadd.u32 q1, q1, q7
vadd.u32 q2, q2, q2
vadd.u32 q2, q2, q7
vstrw.u32 q1, [r0, #16]
vstrw.u32 q2, [r0, #32]
vstrw.u32 q0, [r0], #48
