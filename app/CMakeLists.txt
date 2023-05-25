# For flow puzzle solver

add_executable(solver
	Solver.cpp
	puzzles.cpp
	formations.cpp
	adjacencyLaw.cpp
	deadend.cpp
	oneway.cpp
	RouteGenViaGraph.cpp
	Helper.cpp
)
target_compile_features(solver PRIVATE cxx_std_17)

target_link_libraries(solver PRIVATE flow)
