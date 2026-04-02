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
    explicit LLVMCodeGen(TypeMap type_map);

    void generate(const Program& program);
    void write_ir(const std::string& path);
    int compile_to_object(const std::string& path);
    int compile_to_binary(const std::string& obj_path,
                          const std::string& out_path);

private:
    // Functions & statements (llvm_codegen.cpp)
    void emit_function(const FnDecl& fn);
    void emit_stmt(const Stmt& stmt);
    void emit_let(const LetStmt& stmt);
    void emit_return(const ReturnStmt& stmt);

    // Expressions (llvm_codegen_expr.cpp)
    llvm::Value* emit_expr(const Expr& expr);
    llvm::Value* emit_binary(const BinaryExpr& expr);
    llvm::Value* emit_unary(const UnaryExpr& expr);
    llvm::Value* emit_call(const CallExpr& expr);
    llvm::Value* emit_if(const IfExpr& expr);
    llvm::Value* emit_match(const MatchExpr& expr);
    llvm::Value* emit_block(const BlockExpr& block);
    llvm::Value* emit_print_call(const CallExpr& expr);

    // Helpers
    llvm::Type* llvm_type(AstType type);
    AstType type_of(const Expr& expr) const;
    llvm::Function* get_or_declare_printf();
    llvm::AllocaInst* create_entry_alloca(llvm::Function* fn,
                                           const std::string& name,
                                           llvm::Type* type);

    // State
    llvm::LLVMContext context_;
    std::unique_ptr<llvm::Module> module_;
    llvm::IRBuilder<> builder_;
    std::map<std::string, llvm::AllocaInst*> named_values_;
    TypeMap type_map_;
};
