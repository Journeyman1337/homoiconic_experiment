cmake_minimum_required(VERSION "3.29.3")

project(homoiconic_experiment)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(
    homoiconic_experiment
    "src/main.cpp"

)

#Include(FetchContent)
#FetchContent_Declare(
#  llvm
#  GIT_REPOSITORY https://github.com/llvm/llvm-project
#  GIT_TAG        llvmorg-18.1.7
#)
#FetchContent_MakeAvailable(llvm)

#target_link_libraries(requite PRIVATE llvm)

find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})
llvm_map_components_to_libnames(LLVM_LIBS support core irreader mc mca mcdisassembler mcjit mcparser X86CodeGen X86Info X86Desc TargetParser X86)
target_link_libraries(requite PUBLIC ${LLVM_LIBS})