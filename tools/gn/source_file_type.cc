// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tools/gn/source_file_type.h"

#include "tools/gn/filesystem_utils.h"
#include "tools/gn/source_file.h"

SourceFileType GetSourceFileType(const SourceFile& file) {
  base::StringPiece extension = FindExtension(&file.value());
  if (extension == "cc" || extension == "cpp" || extension == "cxx")
    return SOURCE_CPP;
  if (extension == "h" || extension == "hpp" || extension == "hxx" ||
      extension == "hh")
    return SOURCE_H;
  if (extension == "c")
    return SOURCE_C;
  if (extension == "m")
    return SOURCE_M;
  if (extension == "mm")
    return SOURCE_MM;
  if (extension == "rc")
    return SOURCE_RC;
  if (extension == "S" || extension == "s" || extension == "asm")
    return SOURCE_S;
  if (extension == "o" || extension == "obj")
    return SOURCE_O;
  if (extension == "def")
    return SOURCE_DEF;
  if (extension == "rs")
    return SOURCE_RS;
  if (extension == "go")
    return SOURCE_GO;

  return SOURCE_UNKNOWN;
}

SourceFileTypeSet::SourceFileTypeSet() : empty_(true) {
  memset(flags_, 0, sizeof(bool) * static_cast<int>(SOURCE_NUMTYPES));
}

bool SourceFileTypeSet::CSourceUsed() const {
  return empty_ || Get(SOURCE_CPP) || Get(SOURCE_H) || Get(SOURCE_C) ||
         Get(SOURCE_M) || Get(SOURCE_MM) || Get(SOURCE_RC) || Get(SOURCE_S) ||
         Get(SOURCE_O) || Get(SOURCE_DEF);
}

bool SourceFileTypeSet::RustSourceUsed() const {
  return Get(SOURCE_RS);
}

bool SourceFileTypeSet::GoSourceUsed() const {
  return Get(SOURCE_GO);
}

bool SourceFileTypeSet::MixedSourceUsed() const {
  return (1 << CSourceUsed() << RustSourceUsed() << GoSourceUsed()) > 2;
}
