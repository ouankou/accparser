//===----------------------------------------------------------------------===//
//
// Part of accparser, under the BSD 3-Clause License.
// See LICENSE for license information.
// SPDX-License-Identifier: BSD-3-Clause
//
//===----------------------------------------------------------------------===//

#ifndef ACCPARSER_OPENACCPARSER_H
#define ACCPARSER_OPENACCPARSER_H

#include "OpenACCIR.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace openacc {

struct ParseOptions {
  Language language;
  InputForm inputForm;
};

struct ParseResult {
  std::optional<Directive> directive;
  std::vector<Diagnostic> diagnostics;

  bool succeeded() const;
};

struct PrintOptions {
  std::optional<InputForm> outputForm;
};

ParseResult parseDirective(std::string_view Input, ParseOptions Options);

std::string formatDirective(const Directive &Value, PrintOptions Options = {});

} // namespace openacc

#endif // ACCPARSER_OPENACCPARSER_H
