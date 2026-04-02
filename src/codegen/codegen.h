#pragma once

#include "ast/expr_nodes.h"
#include "ast/stmt_nodes.h"
#include "typechecker/typechecker.h"
#include <sstream>
#include <string>

class CodeGen {
public:
    explicit CodeGen(TypeCheckResult result);
    std::string generate(const Program& program);

private:
    void emit_function(const FnDecl& fn);
    void emit_stmt(const Stmt& stmt);
    void emit_let(const LetStmt& stmt);
    void emit_return(const ReturnStmt& stmt);

    void emit_expr(const Expr& expr);
    void emit_binary(const BinaryExpr& expr);
    void emit_call(const CallExpr& expr);
    void emit_if(const IfExpr& expr);
    void emit_match(const MatchExpr& expr);
    void emit_block(const BlockExpr& block, bool is_top_level);
    void emit_struct_lit(const StructLitExpr& expr);
    void emit_field_access(const FieldAccessExpr& expr);
    void emit_method_call(const MethodCallExpr& expr);
    void emit_static_call(const StaticCallExpr& expr);
    void emit_struct_def(const StructDecl& decl);
    void emit_impl_methods(const ImplBlock& impl);

    void emit(const std::string& code);
    void emit_line(const std::string& code);
    void indent();
    void dedent();
    std::string c_type(AstType type) const;
    std::string c_type_ref(const TypeRef& ref) const;
    AstType type_of(const Expr& expr) const;
    std::string struct_name_of(const Expr& expr) const;
    std::string print_format(AstType type) const;

    std::ostringstream out_;
    int indent_level_ = 0;
    TypeMap type_map_;
    StructNameMap struct_names_;
    StructRegistry registry_;
};
