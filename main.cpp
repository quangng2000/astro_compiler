#include "common/error.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "typechecker/typechecker.h"
#include "codegen/codegen.h"
#include <fstream>
#include <iostream>
#include <sstream>

static std::string read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "error: cannot open file '" << path << "'\n";
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

static int compile_c(const std::string& c_file, const std::string& out_file) {
    auto cmd = "cc -o " + out_file + " " + c_file + " 2>&1";
    int result = std::system(cmd.c_str());
    if (result != 0) {
        std::cerr << "error: C compilation failed\n";
    }
    return result;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: astro <file.astro>\n";
        return 1;
    }

    try {
        auto source = read_file(argv[1]);
        auto tokens = Lexer(source).tokenize();
        auto ast = Parser(std::move(tokens)).parse();
        auto types = TypeChecker().check(ast);
        auto c_code = CodeGen(std::move(types)).generate(ast);

        std::string base(argv[1]);
        auto c_path = base + ".c";
        auto out_path = base.substr(0, base.rfind('.'));

        write_file(c_path, c_code);
        std::cout << "Generated: " << c_path << "\n";

        if (compile_c(c_path, out_path) != 0) return 1;
        std::cout << "Compiled:  " << out_path << "\n";
        return 0;
    } catch (const CompilerError& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
}
