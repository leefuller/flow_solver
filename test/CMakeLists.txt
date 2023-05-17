# Tests need to be added as executables first
add_executable(flowtest test.cpp)

target_compile_features(flowtest PRIVATE cxx_std_17)

target_link_libraries(flowtest PRIVATE flow)

# If you register a test, then ctest and make test will run it.
# You can also run examples and check the output, as well.
add_test(NAME flowlibtest COMMAND flowtest)
