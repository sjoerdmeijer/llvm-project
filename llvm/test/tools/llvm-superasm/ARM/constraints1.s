# RUN: llvm-superasm -mtriple=thumbv8.1-m.main-none-none-eabi -mcpu=cortex-m55 -print-smt-constraints < %s 2> %t
# RUN: cat %t | FileCheck %s --check-prefix=CONSTRAINTS
# RUN: z3 -model %t | FileCheck %s --check-prefix=Z3


# CONSTRAINTS:       (declare-const N0 Int)
# CONSTRAINTS-NEXT:  (assert (> N0 0))
# CONSTRAINTS-NEXT:  (declare-const N1 Int)
# CONSTRAINTS-NEXT:  (assert (> N1 0))
# CONSTRAINTS-NEXT:  (declare-const N2 Int)
# CONSTRAINTS-NEXT:  (assert (> N2 0))
# CONSTRAINTS-NEXT:  (declare-const N3 Int)
# CONSTRAINTS-NEXT:  (assert (> N3 0))
# CONSTRAINTS-NEXT:  (assert (not (or (= N0 N1) (= N0 N2) (= N0 N3) )))
# CONSTRAINTS-NEXT:  (assert (not (or (= N1 N0) (= N1 N2) (= N1 N3) )))
# CONSTRAINTS-NEXT:  (assert (not (or (= N2 N0) (= N2 N1) (= N2 N3) )))
# CONSTRAINTS-NEXT:  (assert (not (or (= N3 N0) (= N3 N1) (= N3 N2) )))
# CONSTRAINTS-NEXT:  (assert (< (+ N0 2) N1))
# CONSTRAINTS-NEXT:  (assert (< (+ N0 2) N1))
# CONSTRAINTS-NEXT:  (minimize (+ N0 N1 N2 N3 ))
# CONSTRAINTS-NEXT:  (check-sat)

# Z3:       sat
# Z3-NEXT:  (
# Z3-NEXT:    (define-fun N1 () Int
# Z3-NEXT:      4)
# Z3-NEXT:    (define-fun N0 () Int
# Z3-NEXT:      1)
# Z3-NEXT:    (define-fun N2 () Int
# Z3-NEXT:      3)
# Z3-NEXT:    (define-fun N3 () Int
# Z3-NEXT:      2)
# Z3-NEXT:  )

mul r1, r2, r3
mul r5, r1, r1
add r4, r2, r3
sub r6, r2, #1
