#pragma once

#include "ast/expr_nodes.h"
#include "ast/stmt_nodes.h"
#include "type_env.h"
#include <unordered_map>

using TypeMap = std::unordered_map<const void*, AstType>;

class TypeChecker {
public:
    TypeMap check(const Program& program);

private:
    AstType check_function(const FnDecl& fn);
    AstType check_expr(const Expr& expr);
    AstType check_block(const BlockExpr& block);
    AstType check_stmt(const Stmt& stmt);
    void expect_type(AstType expected, AstType actual,
                     const std::string& context);
    void record(const Expr& expr, AstType type);

    TypeEnv globals_;
    TypeEnv* current_scope_ = &globals_;
    AstType current_return_type_ = AstType::Void;
    TypeMap type_map_;
};
