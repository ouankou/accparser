#include "OpenACCParser.h"

#include <iostream>

int main() {
  openacc::ParseResult C = openacc::parseDirective(
      "#pragma acc parallel",
      {openacc::Language::C, openacc::InputForm::CPragma});
  openacc::ParseResult Fortran = openacc::parseDirective(
      "!$ACC PARALLEL",
      {openacc::Language::Fortran, openacc::InputForm::FortranFree});

  if (!C.succeeded() || C.directive->language() != openacc::Language::C) {
    std::cerr << "C language was not retained\n";
    return 1;
  }
  if (!Fortran.succeeded() ||
      Fortran.directive->language() != openacc::Language::Fortran) {
    std::cerr << "Fortran language was not retained\n";
    return 1;
  }
  return 0;
}
