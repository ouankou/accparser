# Repository Agent Instructions

- Always consult the latest OpenACC 3.4 specification when updating or extending the parser to ensure semantic accuracy.
- Preserve LLVM coding style across all source files, including C, C++, ANTLR .g4 sources.
- To run the regression tests via CMake/CTest:
  1. Ensure ANTLR4 is installed and visible to CMake (e.g., `which antlr4` should succeed).
  2. Configure the build directory: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug`.
  3. From the `build` directory invoke `ctest --output-on-failure`.
