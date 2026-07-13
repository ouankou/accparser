#include "OpenACCParser.h"
#include "preprocess.h"

#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

namespace {

openacc::ParseOptions optionsFor(PreprocessedForm Form) {
  switch (Form) {
  case PreprocessedForm::CPragma:
    return {openacc::Language::C, openacc::InputForm::CPragma};
  case PreprocessedForm::FortranFree:
    return {openacc::Language::Fortran, openacc::InputForm::FortranFree};
  case PreprocessedForm::FortranFixed:
    return {openacc::Language::Fortran, openacc::InputForm::FortranFixed};
  }
  return {openacc::Language::C, openacc::InputForm::CPragma};
}

} // namespace

int main(int Argc, char **Argv) {
  bool AllowInvalid =
      Argc == 3 && std::string_view(Argv[1]) == "--allow-invalid";
  if ((!AllowInvalid && Argc != 2) || (AllowInvalid && Argc != 3)) {
    std::cerr << "usage: " << Argv[0] << " [--allow-invalid] <test-file>\n";
    return 1;
  }

  const char *InputPath = Argv[AllowInvalid ? 2 : 1];
  std::ifstream InputFile(InputPath);
  if (!InputFile) {
    std::cerr << "could not open " << InputPath << '\n';
    return 1;
  }

  std::string Filename = InputPath;
  Filename = Filename.substr(Filename.find_last_of('/') + 1) + ".output";
  std::ofstream OutputFile(Filename, std::ofstream::trunc);
  if (!OutputFile)
    return 1;

  bool Failed = false;
  for (const PreprocessedDirective &Pragma : preProcess(InputFile)) {
    openacc::ParseOptions Options = optionsFor(Pragma.form);
    openacc::ParseResult Result = openacc::parseDirective(Pragma.text, Options);
    if (!Result.succeeded() || !Result.directive) {
      std::cerr << InputPath << ':' << Pragma.line
                << ": failed to parse directive\n";
      OutputFile << "NULL\n";
      Failed = true;
      continue;
    }
    openacc::PrintOptions PrintOptions;
    PrintOptions.outputForm = Options.inputForm;
    OutputFile << openacc::formatDirective(*Result.directive, PrintOptions)
               << '\n';
  }
  return Failed && !AllowInvalid ? 1 : 0;
}
