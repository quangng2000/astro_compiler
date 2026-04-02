cmake_minimum_required(VERSION 4.0)
project(astro_compiler)

set(CMAKE_CXX_STANDARD 26)

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
)

target_include_directories(astro_compiler PRIVATE src)
