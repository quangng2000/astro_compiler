#pragma once

#include "ast/expr_nodes.h"
#include "ast/stmt_nodes.h"
#include "common/token.h"
#include <vector>

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    Program parse();

private:
    // Declarations & statements (parser_stmt.cpp)
    FnDecl parse_function(bool is_pub = true);
    Stmt parse_statement();
    LetStmt parse_let();
    ReturnStmt parse_return();
    TypeRef parse_type_ref();

    // Struct/impl (parser_struct.cpp)
    StructDecl parse_struct();
    ImplBlock parse_impl();

    // Expressions (parser_expr.cpp)
    ExprPtr parse_expr();
    ExprPtr parse_expr_bp(int min_bp);
    ExprPtr parse_primary();
    ExprPtr parse_if();
    ExprPtr parse_match();
    ExprPtr parse_block();
    std::vector<ExprPtr> parse_arg_list();

    // Postfix (parser_postfix.cpp)
    ExprPtr parse_struct_literal(std::string name);
    ExprPtr apply_postfix(ExprPtr left);
    ExprPtr parse_static_call(std::string type_name);

    // Utilities
    const Token& peek() const;
    const Token& peek_at(size_t offset) const;
    Token advance();
    Token expect(TokenKind kind);
    bool check(TokenKind kind) const;
    bool at_end() const;
    [[noreturn]] void error(const std::string& msg);

    static int infix_bp_left(TokenKind kind);
    static int infix_bp_right(TokenKind kind);
    static int prefix_bp(TokenKind kind);
    static bool is_infix_op(TokenKind kind);

    std::vector<Token> tokens_;
    size_t pos_ = 0;
};
