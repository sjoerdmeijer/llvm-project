//===- LoopInvariantCodeMotion.cpp - Code to perform loop fusion-----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements loop invariant code motion.
//
//===----------------------------------------------------------------------===//

#include "mlir/Transforms/Passes.h"

#include "mlir/IR/PatternMatch.h"
#include "mlir/Interfaces/LoopLikeInterface.h"
#include "mlir/Transforms/LoopInvariantCodeMotionUtils.h"

namespace mlir {
#define GEN_PASS_DEF_LOOPINVARIANTCODEMOTION
#define GEN_PASS_DEF_LOOPINVARIANTSUBSETHOISTING
#include "mlir/Transforms/Passes.h.inc"
} // namespace mlir

using namespace mlir;

namespace {
/// Loop invariant code motion (LICM) pass.
struct LoopInvariantCodeMotion
    : public impl::LoopInvariantCodeMotionBase<LoopInvariantCodeMotion> {
  void runOnOperation() override;
};

struct LoopInvariantSubsetHoisting
    : public impl::LoopInvariantSubsetHoistingBase<
          LoopInvariantSubsetHoisting> {
  void runOnOperation() override;
};
} // namespace

void LoopInvariantCodeMotion::runOnOperation() {
  // Walk through all loops in a function in innermost-loop-first order. This
  // way, we first LICM from the inner loop, and place the ops in
  // the outer loop, which in turn can be further LICM'ed.
  getOperation()->walk(
      [&](LoopLikeOpInterface loopLike) { moveLoopInvariantCode(loopLike); });
}

void LoopInvariantSubsetHoisting::runOnOperation() {
  IRRewriter rewriter(getOperation()->getContext());
  // Walk through all loops in a function in innermost-loop-first order. This
  // way, we first hoist from the inner loop, and place the ops in the outer
  // loop, which in turn can be further hoisted from.
  getOperation()->walk([&](LoopLikeOpInterface loopLike) {
    (void)hoistLoopInvariantSubsets(rewriter, loopLike);
  });
}

std::unique_ptr<Pass> mlir::createLoopInvariantCodeMotionPass() {
  return std::make_unique<LoopInvariantCodeMotion>();
}

std::unique_ptr<Pass> mlir::createLoopInvariantSubsetHoistingPass() {
  return std::make_unique<LoopInvariantSubsetHoisting>();
}
