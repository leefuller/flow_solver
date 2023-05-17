# For flow puzzle solver

add_executable(solver solver.cpp puzzles.cpp)
target_compile_features(solver PRIVATE cxx_std_17)

target_link_libraries(solver PRIVATE flow)
