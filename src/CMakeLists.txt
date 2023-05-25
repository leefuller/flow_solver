# For flow puzzle library

#set(HEADER_LIST "${flow_SOURCE_DIR}/include/*h")
set(SOURCE_LIST 
	${flow_SOURCE_DIR}/src/Cell.cpp
	${flow_SOURCE_DIR}/src/Direction.cpp
	${flow_SOURCE_DIR}/src/Puzzle.cpp
	${flow_SOURCE_DIR}/src/PuzzleDef.cpp
	${flow_SOURCE_DIR}/src/PuzzleException.cpp
	${flow_SOURCE_DIR}/src/PuzzleRepr.cpp
	${flow_SOURCE_DIR}/src/Plumber.cpp
	${flow_SOURCE_DIR}/src/Logger.cpp
)

add_library(flow STATIC Puzzle.cpp ${HEADER_LIST} ${SOURCE_LIST})

target_include_directories(flow PUBLIC ../include)

# This depends on (header only) boost
#target_link_libraries(flow PRIVATE Boost::boost)

# Need at least C++17
target_compile_features(flow PUBLIC cxx_std_17)

# IDEs should put the headers in a nice place
source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})
