enable_testing()

find_package(GTest REQUIRED)
include_directories(
${GTEST_INCLUDE_DIRS}
../include
)

#target_include_directories(flow PUBLIC ../include)

# Tests for CTest need to be added as executables
# To see cout/stdout when running ctest, use ctest --verbose

add_executable(direction_test direction_test.cpp)
target_link_libraries(direction_test 
	PRIVATE flow
	GTest::gtest_main
)

add_executable(cell_test cell_test.cpp)
target_link_libraries(cell_test 
	PRIVATE flow
	GTest::gtest_main
)

add_executable(puzzle_test puzzle_test.cpp)
target_link_libraries(puzzle_test 
	PRIVATE flow
	GTest::gtest_main
)

add_executable(plumber_test plumber_test.cpp)
target_link_libraries(plumber_test
	PRIVATE flow
	GTest::gtest_main
)

add_executable(formations_test formations_test.cpp)
target_sources(formations_test PRIVATE formations_test.cpp ../app/formations.cpp ../app/oneway.cpp ../app/deadend.cpp ../app/adjacencyLaw.cpp)
target_link_libraries(formations_test
	PRIVATE flow
	GTest::gtest_main
)

add_executable(oneway_test oneway_test.cpp)
target_sources(oneway_test PRIVATE oneway_test.cpp ../app/oneway.cpp ../app/adjacencyLaw.cpp)
target_link_libraries(oneway_test
	PRIVATE flow
	GTest::gtest_main
)

add_executable(deadend_test deadend_test.cpp)
target_sources(deadend_test PRIVATE deadend_test.cpp ../app/deadend.cpp)
target_link_libraries(deadend_test
	PRIVATE flow
	GTest::gtest_main
)

add_executable(adjacency_test adjacency_test.cpp)
target_sources(adjacency_test PRIVATE adjacency_test.cpp ../app/adjacencyLaw.cpp ../src/Logger.cpp)
target_link_libraries(adjacency_test 
	PRIVATE flow
	GTest::gtest_main
)

add_executable(graph_test graph_test.cpp)
target_link_libraries(graph_test 
	GTest::gtest_main
)

add_executable(matrix_test matrix_test.cpp)
target_link_libraries(matrix_test 
	GTest::gtest_main
)

add_executable(solver_test solver_test.cpp 
	../app/Solver.cpp ../app/RouteGenViaGraph.cpp ../app/deadend.cpp ../app/formations.cpp ../app/adjacencyLaw.cpp ../app/oneway.cpp)
target_link_libraries(solver_test 
	PRIVATE flow
	GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(direction_test)
gtest_discover_tests(cell_test)
gtest_discover_tests(puzzle_test)
gtest_discover_tests(plumber_test)
gtest_discover_tests(formations_test)
gtest_discover_tests(oneway_test)
gtest_discover_tests(deadend_test)
gtest_discover_tests(adjacency_test)
gtest_discover_tests(graph_test)
gtest_discover_tests(matrix_test)
gtest_discover_tests(solver_test)
