#include "common/error.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "typechecker/typechecker.h"
#include "codegen/codegen.h"
#include "codegen/llvm_codegen.h"
#include <fstream>
#include <iostream>
#include <sstream>

static std::string read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "error: cannot open '" << path << "'\n";
        std::exit(1);
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

static void write_file(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    file << content;
}

static int compile_c(const std::string& c_file, const std::string& out) {
    return std::system(("cc -o " + out + " " + c_file + " 2>&1").c_str());
}

enum class Backend { LLVM, EmitLLVM, EmitC };

static Backend parse_args(int argc, char* argv[], std::string& input) {
    Backend backend = Backend::LLVM;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--emit-llvm") backend = Backend::EmitLLVM;
        else if (arg == "--emit-c") backend = Backend::EmitC;
        else input = arg;
    }
    return backend;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: astro <file.astro> [--emit-llvm|--emit-c]\n";
        return 1;
    }

    std::string input;
    auto backend = parse_args(argc, argv, input);
    if (input.empty()) { std::cerr << "error: no input file\n"; return 1; }

    try {
        auto source = read_file(input);
        auto tokens = Lexer(source).tokenize();
        auto ast = Parser(std::move(tokens)).parse();
        auto types = TypeChecker().check(ast);
        auto base = input.substr(0, input.rfind('.'));

        if (backend == Backend::EmitC) {
            auto c_code = CodeGen(std::move(types)).generate(ast);
            write_file(base + ".c", c_code);
            std::cout << "Generated: " << base << ".c\n";
            if (compile_c(base + ".c", base) != 0) return 1;
            std::cout << "Compiled:  " << base << "\n";
        } else {
            LLVMCodeGen gen(std::move(types));
            gen.generate(ast);
            if (backend == Backend::EmitLLVM) {
                gen.write_ir(base + ".ll");
                std::cout << "Generated: " << base << ".ll\n";
            } else {
                gen.compile_to_object(base + ".o");
                gen.compile_to_binary(base + ".o", base);
                std::cout << "Compiled:  " << base << "\n";
            }
        }
        return 0;
    } catch (const CompilerError& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
}
