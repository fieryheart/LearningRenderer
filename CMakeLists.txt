cmake_minimum_required (VERSION 2.8)
message(STATUS "set compiler")
set(CMAKE_C_COMPILER "/usr/local/Cellar/llvm/12.0.0/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/local/Cellar/llvm/12.0.0/bin/clang++")

project (LearningRenderer)
set(OPENMP_LIBRARIES "/usr/local/opt/llvm@12/lib")
set(OPENMP_INCLUDES "/usr/local/Cellar/llvm/12.0.0/include")

include_directories(${OPENMP_INCLUDES})
link_directories(${OPENMP_LIBRARIES})
link_directories("/usr/local/Cellar/llvm/12.0.0/lib")

include(CheckCXXCompilerFlag)
function(enable_cxx_compiler_flag_if_supported flag)
    string(FIND "${CMAKE_CXX_FLAGS}" "${flag}" flag_already_set)
    if(flag_already_set EQUAL -1)
        check_cxx_compiler_flag("${flag}" flag_supported)
        if(flag_supported)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
        endif()
        unset(flag_supported CACHE)
    endif()
endfunction()

enable_cxx_compiler_flag_if_supported("-Wall")
enable_cxx_compiler_flag_if_supported("-Wextra")
enable_cxx_compiler_flag_if_supported("-pedantic")
enable_cxx_compiler_flag_if_supported("-std=c++17")
enable_cxx_compiler_flag_if_supported("-fopenmp")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

file(GLOB SOURCES *.h *.hpp *.cpp "*/*.h" "*/*.cpp" "*/*.hpp")

add_executable(${PROJECT_NAME} ${SOURCES})

