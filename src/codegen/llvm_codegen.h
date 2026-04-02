#pragma once

#include "ast/expr_nodes.h"
#include "ast/stmt_nodes.h"
#include "typechecker/typechecker.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <map>
#include <string>

class LLVMCodeGen {
public:
    explicit LLVMCodeGen(TypeCheckResult result);

    void generate(const Program& program);
    void write_ir(const std::string& path);
    int compile_to_object(const std::string& path);
    int compile_to_binary(const std::string& obj_path,
                          const std::string& out_path);

private:
    void emit_function(const FnDecl& fn);
    void emit_stmt(const Stmt& stmt);
    void emit_let(const LetStmt& stmt);
    void emit_return(const ReturnStmt& stmt);

    llvm::Value* emit_expr(const Expr& expr);
    llvm::Value* emit_binary(const BinaryExpr& expr);
    llvm::Value* emit_unary(const UnaryExpr& expr);
    llvm::Value* emit_call(const CallExpr& expr);
    llvm::Value* emit_if(const IfExpr& expr);
    llvm::Value* emit_match(const MatchExpr& expr);
    llvm::Value* emit_block(const BlockExpr& block);
    llvm::Value* emit_print_call(const CallExpr& expr);
    llvm::Value* emit_struct_lit(const StructLitExpr& expr);
    llvm::Value* emit_field_access(const FieldAccessExpr& expr);
    llvm::Value* emit_method_call(const MethodCallExpr& expr);
    llvm::Value* emit_static_call(const StaticCallExpr& expr);

    void emit_impl_method(const std::string& target, const ImplMethod& m);
    llvm::Type* llvm_type(AstType type);
    llvm::Type* llvm_type_ref(const TypeRef& ref);
    llvm::StructType* get_struct_type(const std::string& name);
    AstType resolve_type(const TypeRef& ref) const;
    AstType type_of(const Expr& expr) const;
    std::string struct_name_of(const Expr& expr) const;
    llvm::Function* get_or_declare_printf();
    llvm::AllocaInst* create_entry_alloca(llvm::Function* fn,
                                           const std::string& name,
                                           llvm::Type* type);

    llvm::LLVMContext context_;
    std::unique_ptr<llvm::Module> module_;
    llvm::IRBuilder<> builder_;
    std::map<std::string, llvm::AllocaInst*> named_values_;
    std::map<std::string, llvm::StructType*> struct_types_;
    TypeMap type_map_;
    StructNameMap struct_names_;
    StructRegistry registry_;
};
