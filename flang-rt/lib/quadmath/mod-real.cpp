//===-- lib/quadmath/mod-real.cpp -------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "math-entries.h"
#include "numeric-template-specs.h"

namespace Fortran::runtime {
extern "C" {

#if HAS_LDBL128 || HAS_FLOAT128
// MOD (16.9.135)
F128Type RTDEF(ModReal16)(
    F128Type x, F128Type p, const char *sourceFile, int sourceLine) {
  return RealMod<false>(x, p, sourceFile, sourceLine);
}
#endif

} // extern "C"
} // namespace Fortran::runtime
