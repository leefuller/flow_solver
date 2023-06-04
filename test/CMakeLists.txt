
enable_testing()

# Tests for CTest need to be added as executables
# To see cout/stdout when running ctest, use ctest --verbose

add_executable(directiontest direction_test.cpp)
target_link_libraries(directiontest PRIVATE flow)

add_executable(celltest cell_test.cpp)
target_link_libraries(celltest PRIVATE flow)

add_executable(puzzletest puzzle_test.cpp)
target_link_libraries(puzzletest PRIVATE flow)

add_executable(plumbertest plumber_test.cpp)
target_link_libraries(plumbertest PRIVATE flow)

add_executable(formationstest formations_test.cpp)
target_sources(formationstest PRIVATE formations_test.cpp ../app/oneway.cpp ../app/Helper.cpp)
target_link_libraries(formationstest PRIVATE flow)

add_executable(onewaytest oneway_test.cpp)
target_sources(onewaytest PRIVATE oneway_test.cpp ../app/oneway.cpp ../app/adjacencyLaw.cpp ../app/Helper.cpp)
target_link_libraries(onewaytest PRIVATE flow)

add_executable(deadendtest deadend_test.cpp)
target_sources(deadendtest PRIVATE deadend_test.cpp ../app/deadend.cpp ../app/Helper.cpp)
target_link_libraries(deadendtest PRIVATE flow)

add_executable(adjacencytest adjacency_test.cpp)
target_sources(adjacencytest PRIVATE adjacency_test.cpp ../app/adjacencyLaw.cpp ../app/Helper.cpp)
target_link_libraries(adjacencytest PRIVATE flow)

add_executable(graphtest graph_test.cpp)

add_executable(matrixtest matrix_test.cpp)

# Register tests
add_test(NAME directiontest COMMAND directiontest)
add_test(NAME celltest COMMAND celltest)
add_test(NAME puzzletest COMMAND puzzletest)
add_test(NAME plumbertest COMMAND plumbertest)
add_test(NAME graphtest COMMAND graphtest)
add_test(NAME matrixtest COMMAND matrixtest)
add_test(NAME formationstest COMMAND formationstest)
add_test(NAME onewaytest COMMAND onewaytest)
add_test(NAME deadendtest COMMAND deadendtest)
add_test(NAME adjacencytest COMMAND adjacencytest)
