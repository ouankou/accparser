#include "OpenACCParser.h"

#include <iostream>
#include <memory>

int main() {
  openacc::ParseResult Result = openacc::parseDirective(
      "#pragma acc parallel",
      {openacc::Language::Cxx, openacc::InputForm::CPragma});
  if (!Result.succeeded()) {
    std::cerr << "public parser API failed\n";
    return 1;
  }
  if (openacc::formatDirective(*Result.directive) != "#pragma acc parallel") {
    std::cerr << "public formatter API failed\n";
    return 1;
  }

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
  std::unique_ptr<openacc::Directive> FixedForm(
      parseOpenACC("!$acc parallel\n!$acc& copy(a)"));
  std::unique_ptr<openacc::Directive> FreeForm(
      parseOpenACC("!$acc parallel &\n!$acc& copy(a)"));
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

  if (!FixedForm || FixedForm->language() != openacc::Language::Fortran ||
      FixedForm->inputForm() != openacc::InputForm::FortranFixed) {
    std::cerr << "legacy parser did not detect fixed-form continuation\n";
    return 1;
  }
  openacc::PrintOptions BodyOptions;
  BodyOptions.outputForm = openacc::InputForm::DirectiveBody;
  if (openacc::formatDirective(*FixedForm, BodyOptions) != "parallel copy(a)") {
    std::cerr << "legacy parser lost fixed-form continuation payload\n";
    return 1;
  }
  if (!FreeForm || FreeForm->inputForm() != openacc::InputForm::FortranFree) {
    std::cerr << "legacy parser misclassified free-form continuation\n";
    return 1;
  }
  return 0;
}
