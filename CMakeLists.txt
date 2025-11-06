
cmake_minimum_required(VERSION 3.20)
project(accparser)

set(CMAKE_CXX_STANDARD 17)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()

set(ACCPARSER_VERSION_MAJOR 0)
set(ACCPARSER_VERSION_MINOR 1)
set(ACCPARSER_VERSION ${ACCPARSER_VERSION_MAJOR}.${ACCPARSER_VERSION_MINOR})

find_program(ANTLR4_EXECUTABLE NAMES antlr4)
if(NOT ANTLR4_EXECUTABLE)
    message(FATAL_ERROR "antlr4 executable not found. Please install ANTLR4:\n"
                        "  Ubuntu/Debian: sudo apt-get install antlr4\n"
                        "  macOS: brew install antlr\n"
                        "  Or download from: https://www.antlr.org/download.html")
endif()

message(STATUS "Found ANTLR4: ${ANTLR4_EXECUTABLE}")

add_subdirectory(tests)
enable_testing()
set(CMAKE_CTEST_ARGUMENTS --output-on-failure)

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
target_include_directories(accparser SYSTEM PUBLIC
    /usr/include/antlr4-runtime
    )

add_executable(acc_demo.out
    utils/acc_demo.cpp
    tests/preprocess.cpp)
add_dependencies(acc_demo.out accparser)
target_link_libraries(acc_demo.out
    accparser
    antlr4-runtime
    )

add_executable(acc_roundtrip.out
    utils/acc_roundtrip.cpp)
add_dependencies(acc_roundtrip.out accparser)
target_link_libraries(acc_roundtrip.out
    accparser
    antlr4-runtime
    )

install(TARGETS accparser
        LIBRARY DESTINATION lib
        )
install(FILES
        src/OpenACCIR.h
        src/OpenACCKinds.h
        DESTINATION include)
