# For flow puzzle solver

add_executable(solver
	main.cpp
	Solver.cpp
	puzzles.cpp
	formations.cpp
	adjacencyLaw.cpp
	deadend.cpp
	entrapment.cpp
	oneway.cpp
	RouteGenViaGraph.cpp
)
target_compile_features(solver PRIVATE cxx_std_17)

target_link_libraries(solver PRIVATE flow)
