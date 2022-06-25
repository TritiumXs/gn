// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TOOLS_GN_NINJA_TOOLCHAIN_WRITER_H_
#define TOOLS_GN_NINJA_TOOLCHAIN_WRITER_H_

#include <iosfwd>
#include <string>
#include <vector>

#include "gn/ninja_writer.h"
#include "gn/path_output.h"
#include "gn/toolchain.h"

class Settings;

class NinjaToolchainWriter {
 public:
  // Takes the settings for the toolchain, as well as the list of all targets
  // associated with the toolchain.
  static bool RunAndWriteFile(
      const Settings* settings,
      const Toolchain* toolchain,
      const std::vector<NinjaWriter::TargetRulePair>& rules);

 private:
  NinjaToolchainWriter(const Settings* settings,
                       const Toolchain* toolchain,
                       std::ostream& out);
  ~NinjaToolchainWriter();

  void Run(const std::vector<NinjaWriter::TargetRulePair>& extra_rules);

  void WriteRules();

  const Settings* settings_;
  const Toolchain* toolchain_;
  std::ostream& out_;
  PathOutput path_output_;

  NinjaToolchainWriter(const NinjaToolchainWriter&) = delete;
  NinjaToolchainWriter& operator=(const NinjaToolchainWriter&) = delete;
};

#endif  // TOOLS_GN_NINJA_TOOLCHAIN_WRITER_H_