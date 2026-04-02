#pragma once

#include "ast/expr_nodes.h"
#include "ast/stmt_nodes.h"
#include "struct_registry.h"
#include "type_env.h"
#include <unordered_map>

using TypeMap = std::unordered_map<const void*, AstType>;
using StructNameMap = std::unordered_map<const void*, std::string>;

struct TypeCheckResult {
    TypeMap type_map;
    StructNameMap struct_names;
    StructRegistry registry;
};

class TypeChecker {
public:
    TypeCheckResult check(const Program& program);

private:
    void register_structs(const Program& program);
    void register_impls(const Program& program);
    AstType check_function(const FnDecl& fn);
    AstType check_expr(const Expr& expr);
    AstType check_block(const BlockExpr& block);
    AstType check_stmt(const Stmt& stmt);
    AstType check_struct_lit(const StructLitExpr& expr);
    AstType check_field_access(const FieldAccessExpr& expr);
    AstType check_method_call(const MethodCallExpr& expr);
    AstType check_static_call(const StaticCallExpr& expr);
    void expect_type(AstType expected, AstType actual,
                     const std::string& context);
    void record(const Expr& expr, AstType type);
    void record_struct(const Expr& expr, const std::string& name);
    AstType resolve_type(const TypeRef& ref) const;

    TypeEnv globals_;
    TypeEnv* current_scope_ = &globals_;
    AstType current_return_type_ = AstType::Void;
    std::string current_struct_;
    TypeMap type_map_;
    StructNameMap struct_names_;
    StructRegistry registry_;
    std::unordered_map<std::string, std::string> var_struct_names_;
};
