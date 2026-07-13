//===----------------------------------------------------------------------===//
//
// Part of accparser, under the BSD 3-Clause License.
// See LICENSE for license information.
// SPDX-License-Identifier: BSD-3-Clause
//
//===----------------------------------------------------------------------===//

#include "OpenACCParser.h"

#include <cctype>
#include <iostream>
#include <string>
#include <string_view>

namespace {

bool startsWithIgnoringCase(std::string_view Value, std::string_view Prefix) {
  if (Value.size() < Prefix.size())
    return false;
  for (std::size_t I = 0; I < Prefix.size(); ++I)
    if (std::tolower(static_cast<unsigned char>(Value[I])) !=
        std::tolower(static_cast<unsigned char>(Prefix[I])))
      return false;
  return true;
}

openacc::ParseOptions detectOptions(std::string_view Input) {
  std::size_t Begin = Input.find_first_not_of(" \t\r\n");
  if (Begin == std::string_view::npos)
    return {openacc::Language::C, openacc::InputForm::DirectiveBody};
  Input.remove_prefix(Begin);
  if (Input.front() == '#')
    return {openacc::Language::C, openacc::InputForm::CPragma};
  if (Input.rfind("_Pragma", 0) == 0)
    return {openacc::Language::C, openacc::InputForm::CPragmaOperator};
  if (startsWithIgnoringCase(Input, "!$acc"))
    return {openacc::Language::Fortran, openacc::InputForm::FortranFree};
  if (startsWithIgnoringCase(Input, "c$acc") ||
      startsWithIgnoringCase(Input, "*$acc"))
    return {openacc::Language::Fortran, openacc::InputForm::FortranFixed};
  return {openacc::Language::C, openacc::InputForm::DirectiveBody};
}

void printDiagnostics(const openacc::ParseResult &Result) {
  for (const openacc::Diagnostic &Diagnostic : Result.diagnostics)
    std::cerr << Diagnostic.range.begin.line << ':'
              << Diagnostic.range.begin.column << ": " << Diagnostic.message
              << '\n';
}

} // namespace

int main(int Argc, char *Argv[]) {
  if (Argc != 2) {
    std::cerr << "usage: " << Argv[0] << " \"<OpenACC directive>\"\n";
    return 1;
  }

  std::string Input = Argv[1];
  openacc::ParseOptions Options = detectOptions(Input);
  openacc::ParseResult First = openacc::parseDirective(Input, Options);
  if (!First.succeeded()) {
    printDiagnostics(First);
    return 1;
  }

  openacc::PrintOptions PrintOptions;
  PrintOptions.outputForm = Options.inputForm;
  std::string Output = openacc::formatDirective(*First.directive, PrintOptions);
  openacc::ParseResult Second = openacc::parseDirective(Output, Options);
  if (!Second.succeeded() ||
      !First.directive->semanticEquals(*Second.directive)) {
    printDiagnostics(Second);
    std::cerr << "canonical output did not round trip semantically\n";
    return 1;
  }

  std::cout << Output << '\n';
  return 0;
}
