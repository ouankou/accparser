#include "OpenACCParser.h"

#include <cstddef>
#include <cstdint>
#include <string_view>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t *Data,
                                      std::size_t Size) {
  std::string_view Input(reinterpret_cast<const char *>(Data), Size);
  for (openacc::ParseOptions Options :
       {openacc::ParseOptions{openacc::Language::C,
                              openacc::InputForm::DirectiveBody},
        openacc::ParseOptions{openacc::Language::C,
                              openacc::InputForm::CPragma},
        openacc::ParseOptions{openacc::Language::Cxx,
                              openacc::InputForm::CPragmaOperator},
        openacc::ParseOptions{openacc::Language::Fortran,
                              openacc::InputForm::FortranFree},
        openacc::ParseOptions{openacc::Language::Fortran,
                              openacc::InputForm::FortranFixed}}) {
    openacc::ParseResult Result = openacc::parseDirective(Input, Options);
    if (Result.succeeded()) {
      openacc::PrintOptions PrintOptions;
      PrintOptions.outputForm = Options.inputForm;
      (void)openacc::formatDirective(*Result.directive, PrintOptions);
    }
  }
  return 0;
}
