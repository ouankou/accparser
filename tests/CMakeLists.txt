if(NOT TARGET accparser)
    message(FATAL_ERROR "accparser target not found. Ensure accparser library is built before adding tests subdirectory.")
endif()

enable_testing()

add_compile_options(-Wall)

add_executable(acc_tester.out
    acc_tester.cpp
    preprocess.cpp)
add_dependencies(acc_tester.out accparser)
target_link_libraries(acc_tester.out accparser)

add_executable(acc_roundtrip.out
    acc_roundtrip.cpp)
add_dependencies(acc_roundtrip.out accparser)
target_link_libraries(acc_roundtrip.out accparser)

add_executable(lang_flag_test.out
    lang_flag_test.cpp)
add_dependencies(lang_flag_test.out accparser)
target_link_libraries(lang_flag_test.out accparser)

add_test(NAME lang_flag_test COMMAND lang_flag_test.out)

add_executable(compat_caller.out
    compat_caller.cpp)
add_dependencies(compat_caller.out accparser)
target_link_libraries(compat_caller.out accparser)

add_test(NAME compat_caller COMMAND compat_caller.out)

find_package(Threads REQUIRED)
add_executable(parser_tests.out parser_tests.cpp)
add_dependencies(parser_tests.out accparser)
target_link_libraries(parser_tests.out accparser Threads::Threads)
add_test(NAME parser_tests COMMAND parser_tests.out)

add_executable(preprocess_tests.out preprocess_tests.cpp preprocess.cpp)
add_test(NAME preprocess_tests COMMAND preprocess_tests.out)

# File-based snapshots cover canonical output and expected-invalid NULL rows.
add_subdirectory(builtin)

if(ACCPARSER_BUILD_FUZZER)
  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "ACCPARSER_BUILD_FUZZER requires Clang")
  endif()
  add_executable(parser_fuzz parser_fuzz.cpp)
  add_dependencies(parser_fuzz accparser)
  target_compile_options(parser_fuzz PRIVATE -fsanitize=fuzzer,address,undefined)
  target_link_options(parser_fuzz PRIVATE -fsanitize=fuzzer,address,undefined)
  target_link_libraries(parser_fuzz PRIVATE accparser)
endif()

# Register OpenACC VV extracted pragma files as CTest tests
file(GLOB_RECURSE OPENACC_VV_FILES
     "${CMAKE_CURRENT_SOURCE_DIR}/openacc_vv/*.c"
     "${CMAKE_CURRENT_SOURCE_DIR}/openacc_vv/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/openacc_vv/*.cc"
     "${CMAKE_CURRENT_SOURCE_DIR}/openacc_vv/*.cxx"
     "${CMAKE_CURRENT_SOURCE_DIR}/openacc_vv/*.f"
     "${CMAKE_CURRENT_SOURCE_DIR}/openacc_vv/*.f90"
     "${CMAKE_CURRENT_SOURCE_DIR}/openacc_vv/*.f95"
     "${CMAKE_CURRENT_SOURCE_DIR}/openacc_vv/*.F"
     "${CMAKE_CURRENT_SOURCE_DIR}/openacc_vv/*.F90")
list(SORT OPENACC_VV_FILES)
foreach(TEST_FILE IN LISTS OPENACC_VV_FILES)
  # Get relative path from tests/openacc_vv/ to include directory structure in test name
  file(RELATIVE_PATH TEST_REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/openacc_vv" "${TEST_FILE}")
  # Replace directory separators with underscores for test name
  string(REPLACE "/" "_" TEST_NAME "${TEST_REL_PATH}")
  add_test(NAME openacc_vv_${TEST_NAME}
           COMMAND "${CMAKE_COMMAND}" -E env
                   "LD_LIBRARY_PATH=$<TARGET_FILE_DIR:accparser>:$ENV{LD_LIBRARY_PATH}"
                   "${CMAKE_CURRENT_SOURCE_DIR}/test_single_pragma.sh"
                   "${TEST_FILE}" "$<TARGET_FILE:acc_roundtrip.out>"
           WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
endforeach()

set(ACCPARSER_DIRECT_TESTS
    lang_flag_test
    compat_caller
    parser_tests
    preprocess_tests)
foreach(TEST_NAME IN LISTS ACCPARSER_DIRECT_TESTS)
  set_property(TEST ${TEST_NAME} PROPERTY ENVIRONMENT
               "LD_LIBRARY_PATH=$<TARGET_FILE_DIR:accparser>:$ENV{LD_LIBRARY_PATH}")
endforeach()

set(accparser_targets accparser)
set(executable_targets
    acc_tester.out
    )

install(TARGETS ${executable_targets}
        RUNTIME DESTINATION bin
        )
