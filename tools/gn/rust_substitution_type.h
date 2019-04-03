// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TOOLS_GN_RUST_SUBSTITUTION_TYPE_H_
#define TOOLS_GN_RUST_SUBSTITUTION_TYPE_H_

#include <set>
#include <vector>

#include "tools/gn/substitution_type.h"

// The set of substitutions available to all tools.
extern const SubstitutionTypes RustSubstitutions;

// Valid for compiler tools.
extern const Substitution RustSubstitutionRustFlags;
extern const Substitution RustSubstitutionRustEnv;
extern const Substitution RustSubstitutionCrateName;
extern const Substitution RustSubstitutionRustDeps;
extern const Substitution RustSubstitutionRustRlibs;

bool IsValidRustSubstitution(const Substitution* type);

#endif  // TOOLS_GN_RUST_SUBSTITUTION_TYPE_H_
