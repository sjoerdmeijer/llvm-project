# RUN: llvm-superasm -mtriple=thumbv8.1-m.main-none-none-eabi -mcpu=cortex-m55 < %s 2> %t

# CHECK:            mul     r1, r2, r3
# CHECK-NEXT:       sub.w   r6, r2, #1
# CHECK-NEXT:       add.w   r4, r2, r3
# CHECK-NEXT:       mul     r5, r1, r1

mul r1, r2, r3
mul r5, r1, r1
add r4, r2, r3
sub r6, r2, #1
