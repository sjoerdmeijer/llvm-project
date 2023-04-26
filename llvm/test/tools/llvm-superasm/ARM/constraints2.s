# RUN: llvm-superasm -mtriple=thumbv8.1-m.main-none-none-eabi -mcpu=cortex-m55 -print-solver-model < %s 2> %t

# CHECK:       Solver Model:
# CHECK:       N1 -> 3
# CHECK-NEXT:  N0 -> 1
# CHECK-NEXT:  N2 -> 4
# CHECK-NEXT:  N3 -> 2

mul r1, r2, r3
mul r5, r1, r1
add r4, r2, r3
sub r6, r2, #1
