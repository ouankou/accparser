//===----------------------------------------------------------------------===//
//
// Part of accparser, under the BSD 3-Clause License.
// See LICENSE for license information.
// SPDX-License-Identifier: BSD-3-Clause
//
//===----------------------------------------------------------------------===//

#include "preprocess.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main() {
  std::istringstream Input("#pragma accx vendor_hint\n"
                           "#pragma acc parallel\n"
                           "#pragma access vendor_hint\n"
                           "!$accx parallel\n"
                           "!$acc serial\n"
                           "integer :: gap\n"
                           "c$accx parallel\n"
                           "c$acc serial\n"
                           "!$acc parallel\n"
                           "!$acc& copy(a)\n"
                           "!$acc kernels &\n"
                           "!$acc ! ignored continuation comment\n"
                           "!$acc& copy(b)\n"
                           "!$acc ! ignored standalone comment\n"
                           "  !$ACC\t! also ignored\n"
                           "!$acc parallel ! ordinary inline comment\n");
  std::vector<PreprocessedDirective> Expected = {
      {"#pragma acc parallel", 2, PreprocessedForm::CPragma},
      {"!$acc serial", 5, PreprocessedForm::FortranFree},
      {"c$acc serial", 8, PreprocessedForm::FortranFixed},
      {"!$acc parallel\n!$acc& copy(a)", 9, PreprocessedForm::FortranFixed},
      {"!$acc kernels &\n!$acc ! ignored continuation comment\n"
       "!$acc& copy(b)",
       11, PreprocessedForm::FortranFree},
      {"!$acc parallel ! ordinary inline comment", 16,
       PreprocessedForm::FortranFree},
  };
  std::vector<PreprocessedDirective> Actual = preProcess(Input);
  if (Actual == Expected)
    return 0;

  std::cerr << "OpenACC directive discovery or continuation handling failed\n";
  for (const PreprocessedDirective &Directive : Actual)
    std::cerr << Directive.line << ": " << Directive.text << '\n';
  return 1;
}
