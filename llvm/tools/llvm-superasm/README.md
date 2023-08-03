# Introduction
`llvm-superasm` consumes assembly code and produces super optimised assembly
code. Super optimisation is in the name because it produces optimal results
using a SAT solver. This first version and proof-of-concept focuses on
instruction scheduling. I.e., the instruction scheduling problem is formulated
as a constraint set and solved by the Z3 SAT solver, and this first version
implements that for a single basic block.

Next, I would like to add and implement other optimisations: i) register
allocation/renaming, and ii) software pipeling as described
[here](https://eprint.iacr.org/2022/1303.pdf) . `llvm-superasm` is a
reimplementation of the ideas in that work but is leveraging everything that
the llvm-project has to offer in this area: option parsing, assembly parsing,
architecture and instruction descriptions, instruction printers, etc. For
instruction scheduling and this proof of concept implementation, it is using
the instruction and latency descriptions to calculate a better schedule if
there’s one. I see two main use-cases for this such an assembly super optimiser
tool:
- Some people write optimised routines in assembly, e.g. library writers, and
  this tool could help with that.
- A post-processing tool for assembly produced by other tools, e.g. compilers.

This has been discussed as an RFC on LLVM discourse
[here](https://discourse.llvm.org/t/rfc-assembly-super-optimiser/71365).
The prototype with all the commits squashed can be found on Phrabricator 
[here](https://reviews.llvm.org/D152949).

# Building

To build `llvm-superasm`, the `llvm-project` needs to be configured with
`-DLLVM_ENABLE_Z3_SOLVER=ON` and `-DLLVM_Z3_INSTALL_DIR`. To run the
regression tests, the `ARM` back-end needs to be build. Thus, the configure
and build commands could e.g. look like this:

```
cd llvm-project
mkdir build
cd build
cmake ../llvm -G Ninja -DLLVM_ENABLE_PROJECTS="clang" -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD="AArch64;ARM" -DLLVM_ENABLE_Z3_SOLVER=ON -DLLVM_Z3_INSTALL_DIR=/path/to/z3
ninja llvm-superasm
```

# Running

The regression tests in `llvm-project/llvm/test/tools/llvm-superasm/ARM` are a
good inspiration how things can be run, e.g.:

```
llvm-superasm -mtriple=thumbv8.1-m.main-none-none-eabi -mcpu=cortex-m55  llvm-project/llvm/test/tools/llvm-superasm/ARM/scheduling1.s
```

# Compilation Flow

The tool in its current form is in essence a driver for certain LLVM libraries
that are responsible for assembly parsing, retrieving architecture and
instructions descriptions, and the Z3 SAT solver. The Z3 solver is already part
of the LLVM project as it can be used by one of the sanitizers. As input, the
tool consumes assembly code, for example:

```
mul r1, r2, r3
mul r5, r1, r1
add r4, r2, r3
sub r6, r2, #1
```

A dependence graph is created and can be printed in Graphviz format as a debug
feature. For this example, we have 4 nodes corresponding to the 4 program
assembly statements, and 1 edge corresponding to the true-dependency between
the two MUL instructions:

```
digraph deps {
  N0 [label = "   mul     r1, r2, r3"];
  N1 [label = "   mul     r5, r1, r1"];
  N2 [label = "   add.w   r4, r2, r3"];
  N3 [label = "   sub.w   r6, r2, #1"];
  N0 -> N1 [label = "R1:true"];
}
```

If a MUL instruction has a latency of 2 cycles, then the ADD and SUB
instructions can be scheduled in between the two dependent MULs to hide the
latency and avoid pipeline stalls. The dependence graph combined with these
instructions latencies, are translated to a constraint set, which is setup to
find a minimum schedule. This is the corresponding constraint system in text
format that serves as input to the Z3 solver:

```
(declare-const N0 Int)
(assert (> N0 0))
(declare-const N1 Int)
(assert (> N1 0))
(declare-const N2 Int)
(assert (> N2 0))
(declare-const N3 Int)
(assert (> N3 0))
(assert (not (or (= N0 N1) (= N0 N2) (= N0 N3) )))
(assert (not (or (= N1 N2) (= N1 N3) )))
(assert (not (or (= N2 N3) )))
(assert (< (+ N0 2) N1))
(minimize (+ N0 N1 N2 N3 ))
(check-sat)
```

As a debug feature, the tool allows to dump the constraints in Z3 textual
format shown above, but to query the Z3 solver the tool uses its C-API. The
constraints system above specifies that there are 4 variables corresponding to
the 4 statements. The values of these variables will corresponds to the new
position of the instruction in the instruction sequence. The constraints above
specify that these positions should be greater than 0, should not be equal to
each other, and the position of N0 should be two less than N1 because of the
dependency and latency. Finally, we specify that the minimum values for these
variables should be found. The solver finds a solution by assigning integer
variables to the nodes. For this example, the solver will finds the following
solution:

```
N0 -> 1
N1 -> 4
N2 -> 3
N3 -> 2
```

As mentioned, these integer values correspond to their new position in the
instruction sequence. So, the first node gets position 1 in the new schedule
and the second node gets position 4, etc. And what we thus achieve, it that the
two MUL instruction will be scheduled apart. Finally, this assignment is
translated to a new schedule:

```
mul     r1, r2, r3
sub.w   r6, r2, #1
add.w   r4, r2, r3
mul     r5, r1, r1
```
