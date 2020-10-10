cmake_minimum_required(VERSION 3.1)
project(omega)

set(CMAKE_CXX_STANDARD 11)

enable_testing()

set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/build CACHE STRING "set install prefix")
install(CODE "MESSAGE(\"Installing into ${CMAKE_INSTALL_PREFIX}\")")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR}/include)

# install al header files
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ohm
        DESTINATION include)

# project files for clion
FILE(GLOB_RECURSE TEST_FILES ${PROJECT_SOURCE_DIR}/test/*.cpp)
FILE(GLOB_RECURSE TOOL_FILES ${PROJECT_SOURCE_DIR}/tools/*.cpp)
FILE(GLOB_RECURSE CTEST_FILES ${PROJECT_SOURCE_DIR}/ctest/*.cpp)

# example for platform and toolchain
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR
        "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "NVIDIA")
endif()

if (MINGW)
    link_libraries(wsock32 ws2_32)
elseif(WIN32)
elseif (ANDROID)
elseif (IOS)
elseif (APPLE)
elseif (UNIX)
    link_libraries(pthread)
else ()
endif ()

# build test codes
foreach (path ${TEST_FILES})
    string(REGEX MATCH "[^/]*.[(c)|(cc)|(cpp)]$" file_ext ${path})
    string(REGEX MATCH "^[^.]*" file ${file_ext})
    add_executable(test_${file} ${path})
    set_target_properties(test_${file} PROPERTIES OUTPUT_NAME test_${file})
endforeach ()

# build tools
foreach (path ${TOOL_FILES})
    string(REGEX MATCH "[^/]*.[(c)|(cc)|(cpp)]$" file_ext ${path})
    string(REGEX MATCH "^[^.]*" file ${file_ext})
    add_executable(tool_${file} ${path})
    set_target_properties(tool_${file} PROPERTIES OUTPUT_NAME ${file})
    install(TARGETS tool_${file}
            RUNTIME DESTINATION bin)
endforeach ()

# build ctest codes
foreach (path ${CTEST_FILES})
    string(REGEX MATCH "[^/]*.[(c)|(cc)|(cpp)]$" file_ext ${path})
    string(REGEX MATCH "^[^.]*" file ${file_ext})
    add_executable(ctest_${file} ${path})
    set_target_properties(ctest_${file} PROPERTIES OUTPUT_NAME ctest_${file})
    add_test(NAME run_ctest_${file} COMMAND ctest_${file})
endforeach ()