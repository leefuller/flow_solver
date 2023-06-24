set(DOXYGEN_EXTRACT_ALL YES)
set(DOXYGEN_BUILTIN_STL_SUPPORT YES)

set(DOXYGEN_GENERATE_HTML YES)
set(DOXYGEN_GENERATE_MAN NO)

set(DOXYGEN_EXCLUDE_PATTERNS 
        ${PROJECT_SOURCE_DIR}/test
        ${PROJECT_SOURCE_DIR}/include/Logger.h
        ${PROJECT_SOURCE_DIR}/src/Logger.cpp)

message(STATUS "Configure doxygen")

doxygen_add_docs(
    docs
    ${PROJECT_SOURCE_DIR}
    ALL
    COMMENT "Generate docs"
)

