# accparser

`accparser` is a standalone OpenACC 3.4 directive parser for C, C++, and
Fortran. It separates the directive envelope and OpenACC structure from opaque
host-language expressions, then produces an owning, value-semantic typed AST.
Invalid input produces diagnostics and never produces a partial AST.

## Prerequisites

The build requires CMake 3.20 or later, ANTLR 4, and the ANTLR 4 C++ runtime.
On Ubuntu or Debian:

```bash
sudo apt update
sudo apt install -y antlr4 libantlr4-runtime-dev build-essential cmake
```

## Build and test

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
cd build
ctest --output-on-failure
```

The regression suite includes focused positive and negative parser tests,
concurrent parsing, semantic parse-print-parse checks, and pragmas extracted
from OpenACCV-V.

Optional hardening configurations are available:

```bash
cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug \
  -DACCPARSER_ENABLE_ASAN_UBSAN=ON
cmake -S . -B build-tsan -DCMAKE_BUILD_TYPE=Debug \
  -DACCPARSER_ENABLE_TSAN=ON
```

With Clang, `-DACCPARSER_BUILD_FUZZER=ON` also builds `parser_fuzz`. A seed
corpus is provided in `tests/fuzz_corpus`.

## Public API

Include `OpenACCParser.h`; public headers do not expose generated ANTLR types.
The caller specifies the base language and source form explicitly:

```cpp
#include "OpenACCParser.h"

openacc::ParseResult result = openacc::parseDirective(
    "#pragma acc parallel loop gang vector private(a)",
    {openacc::Language::Cxx, openacc::InputForm::CPragma});

if (!result.succeeded()) {
  for (const openacc::Diagnostic &diagnostic : result.diagnostics) {
    // diagnostic.code, diagnostic.message, and diagnostic.range are available.
  }
  return;
}

std::string canonical = openacc::formatDirective(*result.directive);
```

Supported input forms are a directive body, a C/C++ `#pragma`, a C/C++
`_Pragma` operator, and free- or fixed-form Fortran sentinels. The formatter can
emit any selected envelope through `openacc::PrintOptions`; by default it emits
`#pragma` for C/C++ and a free-form sentinel for Fortran.

The AST owns its data and uses clause-specific types. For example, `copyin`,
`copyout`, and `create` have distinct modifier enums, device-specific clauses
are partitioned into explicit groups, lists are nonempty by construction, and
host fragments are distinguished as conditions, integer expressions, variable
references, routine names, and other semantic categories.

`parseOpenACC(std::string)` remains as a deprecated transition wrapper. New
code should use `openacc::parseDirective`, which is reentrant, has no global
merge mode, and returns structured diagnostics.
