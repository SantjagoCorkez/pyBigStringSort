cmake_minimum_required(VERSION 3.21)
project(pyBigStringSort C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_SHARED_LIBRARY_PREFIX "")

FIND_PACKAGE(Python3 COMPONENTS Development)
include_directories(${Python3_INCLUDE_DIRS})
link_directories(${Python3_LIBRARY_DIRS})
link_libraries(${Python3_LIBRARIES})

add_compile_definitions(
        _Py_BigStringSort_PyExec=${Python3_EXECUTABLE}
)

# The rule is simple:
# - library name must retain on a separate line
# - SHARED keyword must retain on a separate line
# - each source file must be the only on its line and must not share the same line with keywords and brackets
# - closing bracket must be on a separate line and must be the only symbol in that line
#  Violating this rule will cause `setup.py` to fail to build the proper sources list for Extension
add_library(
        big_string_sort.${Python3_SOABI}
        SHARED
        library.c
)
