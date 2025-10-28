enable_testing()
add_subdirectory(base)

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

set(accparser_targets accparser)
set(executable_targets
    acc_tester.out
    )

install(TARGETS ${executable_targets}
        RUNTIME DESTINATION bin
        )
