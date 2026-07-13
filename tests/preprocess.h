//===----------------------------------------------------------------------===//
//
// Part of accparser, under the BSD 3-Clause License.
// See LICENSE for license information.
// SPDX-License-Identifier: BSD-3-Clause
//
//===----------------------------------------------------------------------===//

#ifndef ACCPARSER_TESTS_PREPROCESS_H
#define ACCPARSER_TESTS_PREPROCESS_H

#include <istream>
#include <string>
#include <vector>

enum class PreprocessedForm { CPragma, FortranFree, FortranFixed };

struct PreprocessedDirective {
  std::string text;
  int line;
  PreprocessedForm form;

  bool operator==(const PreprocessedDirective &Other) const {
    return text == Other.text && line == Other.line && form == Other.form;
  }
};

std::vector<PreprocessedDirective> preProcess(std::istream &InputFile);

#endif // ACCPARSER_TESTS_PREPROCESS_H
