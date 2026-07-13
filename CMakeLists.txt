
cmake_minimum_required(VERSION 3.20)
project(accparser VERSION 0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()

option(ACCPARSER_ENABLE_ASAN_UBSAN
       "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(ACCPARSER_ENABLE_TSAN "Build with ThreadSanitizer" OFF)
option(ACCPARSER_BUILD_FUZZER "Build the libFuzzer parser target" OFF)

if(ACCPARSER_ENABLE_ASAN_UBSAN AND ACCPARSER_ENABLE_TSAN)
    message(FATAL_ERROR "ASan/UBSan and TSan cannot be enabled together")
endif()
if(ACCPARSER_ENABLE_ASAN_UBSAN)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
elseif(ACCPARSER_ENABLE_TSAN)
    add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
    add_link_options(-fsanitize=thread)
endif()

find_program(ANTLR4_EXECUTABLE NAMES antlr4)
if(NOT ANTLR4_EXECUTABLE)
    message(FATAL_ERROR "antlr4 executable not found. Please install ANTLR4:\n"
                        "  Ubuntu/Debian: sudo apt-get install antlr4\n"
                        "  macOS: brew install antlr\n"
                        "  Or download from: https://www.antlr.org/download.html")
endif()

message(STATUS "Found ANTLR4: ${ANTLR4_EXECUTABLE}")

set(ACCPARSER_GRAMMAR_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/acclexer.g4
    ${CMAKE_CURRENT_SOURCE_DIR}/src/accparser.g4
    )

set(ACCIR_SOURCE_FILES
    src/OpenACCASTConstructor.cpp
    src/OpenACCIR.cpp
    src/OpenACCIRToString.cpp
    )

set(ACCPARSER_GRAMMAR_TARGET_FILES
  acclexer.cpp
  accparser.cpp
)

add_custom_command(OUTPUT
    ${ACCPARSER_GRAMMAR_TARGET_FILES}
    COMMAND
    ${ANTLR4_EXECUTABLE} ${ACCPARSER_GRAMMAR_FILES} -Werror -Dlanguage=Cpp -listener -visitor -o ${CMAKE_CURRENT_BINARY_DIR}
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    DEPENDS "${ACCPARSER_GRAMMAR_FILES}"
    )

add_custom_target(accparser_grammar DEPENDS ${ACCPARSER_GRAMMAR_TARGET_FILES})

add_compile_options(-Wall)

add_library(accparser SHARED ${ACCIR_SOURCE_FILES} ${ACCPARSER_GRAMMAR_TARGET_FILES})
add_dependencies(accparser accparser_grammar)
target_link_libraries(accparser
    antlr4-runtime
    )
target_include_directories(accparser PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

if(NOT TARGET antlr4_shared AND NOT TARGET antlr4_static)
    find_path(ANTLR4_INCLUDE_DIR antlr4-runtime.h
        PATHS /usr/include/antlr4-runtime /usr/local/include/antlr4-runtime
        DOC "ANTLR4 C++ runtime include directory")
    if(ANTLR4_INCLUDE_DIR)
        target_include_directories(accparser SYSTEM PUBLIC ${ANTLR4_INCLUDE_DIR})
    endif()
endif()

add_subdirectory(tests)
enable_testing()
set(CMAKE_CTEST_ARGUMENTS --output-on-failure)

install(TARGETS accparser
        LIBRARY DESTINATION lib
        )

# Install public headers
# OpenACCParser.h: Standalone parser interface (no ANTLR dependency)
# OpenACCIR.h: AST node classes
# OpenACCKinds.h: Enum definitions
install(FILES
        src/OpenACCParser.h
        src/OpenACCIR.h
        src/OpenACCKinds.h
        DESTINATION include)
