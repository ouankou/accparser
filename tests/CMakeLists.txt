enable_testing()
add_subdirectory(builtin)

add_compile_options(-Wall)

add_executable(acc_tester.out
    acc_tester.cpp
    preprocess.cpp)
add_dependencies(acc_tester.out accparser)
target_link_libraries(acc_tester.out
    accparser
    antlr4-runtime
    )

add_executable(lang_flag_test.out
    lang_flag_test.cpp)
add_dependencies(lang_flag_test.out accparser)
target_link_libraries(lang_flag_test.out
    accparser
    antlr4-runtime
)

add_test(NAME lang_flag_test COMMAND lang_flag_test.out)

add_executable(compat_caller.out
    compat_caller.cpp)
add_dependencies(compat_caller.out accparser)
target_link_libraries(compat_caller.out
    accparser
    antlr4-runtime
)

add_test(NAME compat_caller COMMAND compat_caller.out)

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
           COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/test_single_pragma.sh" "${TEST_FILE}" "$<TARGET_FILE:acc_roundtrip.out>"
           WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
endforeach()

set(accparser_targets accparser)
set(executable_targets
    acc_tester.out
    )

install(TARGETS ${executable_targets}
        RUNTIME DESTINATION bin
        )
