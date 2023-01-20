// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TOOLS_GN_RESOLVED_TARGET_DATA_H_
#define TOOLS_GN_RESOLVED_TARGET_DATA_H_

#include <memory>
#include <vector>

#include "base/containers/span.h"
#include "gn/lib_file.h"
#include "gn/resolved_target_deps.h"
#include "gn/source_dir.h"
#include "gn/target.h"
#include "gn/target_public_pair.h"
#include "gn/unique_vector.h"

// A class used to compute target-specific data by collecting information
// from its tree of dependencies.
//
// For example, linkable targets can call GetLinkedLibraries() and
// GetLinkedLibraryDirs() to find the library files and library search
// paths to add to their final linker command string, based on the
// definitions of the `libs` and `lib_dirs` config values of their
// transitive dependencies.
//
// Values are computed on demand, but memorized by the class instance in order
// to speed up multiple queries for targets that share dependencies.
//
// Usage is:
//
//  1) Create instance.
//
//  2) Call any of the methods to retrieve the value of the corresponding
//     data. For all methods, the input Target instance passed as argument
//     must have been fully resolved (meaning that Target::OnResolved()
//     must have been called and completed). Input target pointers are
//     const and thus are never modified. This allows using multiple
//     ResolvedTargetData instances from the same input graph in multiple
//     threads safely.
//
class ResolvedTargetData {
 public:
  // Return the public/private/data/dependencies of a given target
  // as a ResolvedTargetDeps instance.
  const ResolvedTargetDeps& GetTargetDeps(const Target* target) const {
    return GetTargetInfo(target)->deps;
  }

  // Return the data dependencies of a given target.
  // Convenience shortcut for GetTargetDeps(target).data_deps().
  base::span<const Target*> GetDataDeps(const Target* target) const {
    return GetTargetDeps(target).data_deps();
  }

  // Return the public and private dependencies of a given target.
  // Convenience shortcut for GetTargetDeps(target).linked_deps().
  base::span<const Target*> GetLinkedDeps(const Target* target) const {
    return GetTargetDeps(target).linked_deps();
  }

  // The list of all library directory search path to add to the final link
  // command of linkable binary. For example, if this returns ['dir1', 'dir2']
  // a command for a C++ linker would typically use `-Ldir1 -Ldir2`.
  const std::vector<SourceDir>& GetLinkedLibraryDirs(
      const Target* target) const {
    return GetTargetLibInfo(target)->lib_dirs;
  }

  // The list of all library files to add to the final link command of linkable
  // binaries. For example, if this return s['foo', '/path/to/bar'], the command
  // for a C++ linker would typically use '-lfoo /path/to/bar'.
  const std::vector<LibFile>& GetLinkedLibraries(const Target* target) const {
    return GetTargetLibInfo(target)->libs;
  }

 private:
  // The information associated with a given Target pointer.
  struct TargetInfo {
    TargetInfo() = default;

    TargetInfo(const Target* target)
        : target(target),
          deps(target->public_deps(),
               target->private_deps(),
               target->data_deps()) {}

    const Target* target = nullptr;
    ResolvedTargetDeps deps;

    bool has_lib_info = false;

    // Only valid if |has_lib_info|.
    std::vector<SourceDir> lib_dirs;
    std::vector<LibFile> libs;
  };

  // Retrieve TargetInfo value associated with |target|. Create
  // a new empty instance on demand if none is already available.
  TargetInfo* GetTargetInfo(const Target* target) const;

  const TargetInfo* GetTargetLibInfo(const Target* target) const {
    TargetInfo* info = GetTargetInfo(target);
    if (!info->has_lib_info) {
      ComputeLibInfo(info);
      DCHECK(info->has_lib_info);
    }
    return info;
  }

  // Compute the portion of TargetInfo guarded by one of the |has_xxx|
  // booleans. This performs recursive and expensive computations and
  // should only be called once per TargetInfo instance.
  void ComputeLibInfo(TargetInfo* info) const;

  // A { Target* -> TargetInfo } map that will create entries
  // on demand (hence the mutable qualifier). Implemented with a
  // UniqueVector<> and a parallel vector of unique TargetInfo
  // instances for best performance.
  mutable UniqueVector<const Target*> targets_;
  mutable std::vector<std::unique_ptr<TargetInfo>> infos_;
};

#endif  // TOOLS_GN_RESOLVED_TARGET_DATA_H_
