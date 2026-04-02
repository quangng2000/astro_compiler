cmake_minimum_required(VERSION 4.0)
project(astro_compiler)

set(CMAKE_CXX_STANDARD 26)

set(LLVM_DIR "/opt/homebrew/opt/llvm/lib/cmake/llvm")
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

llvm_map_components_to_libnames(llvm_libs core support native passes)

add_executable(astro_compiler
    main.cpp
    src/lexer/lexer.cpp
    src/lexer/lexer_helpers.cpp
    src/parser/parser_stmt.cpp
    src/parser/parser_expr.cpp
    src/parser/parser_helpers.cpp
    src/typechecker/typechecker.cpp
    src/codegen/codegen_stmt.cpp
    src/codegen/codegen_expr.cpp
    src/codegen/llvm_codegen.cpp
    src/codegen/llvm_codegen_expr.cpp
    src/codegen/llvm_codegen_emit.cpp
)

target_include_directories(astro_compiler PRIVATE src ${LLVM_INCLUDE_DIRS})
target_link_libraries(astro_compiler PRIVATE ${llvm_libs})
