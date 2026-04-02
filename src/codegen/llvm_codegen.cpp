#include "llvm_codegen.h"
#include "common/error.h"
#include <llvm/IR/Verifier.h>

LLVMCodeGen::LLVMCodeGen(TypeMap type_map)
    : module_(std::make_unique<llvm::Module>("astro", context_))
    , builder_(context_)
    , type_map_(std::move(type_map)) {}

void LLVMCodeGen::generate(const Program& program) {
    for (auto& fn : program.functions) emit_function(fn);
    llvm::verifyModule(*module_, &llvm::errs());
}

void LLVMCodeGen::emit_function(const FnDecl& fn) {
    std::vector<llvm::Type*> param_types;
    for (auto& p : fn.params) param_types.push_back(llvm_type(p.type));

    auto* ret_type = llvm_type(fn.return_type);
    auto* ft = llvm::FunctionType::get(ret_type, param_types, false);
    auto* func = llvm::Function::Create(
        ft, llvm::Function::ExternalLinkage, fn.name, module_.get());

    auto* entry = llvm::BasicBlock::Create(context_, "entry", func);
    builder_.SetInsertPoint(entry);
    named_values_.clear();

    size_t i = 0;
    for (auto& arg : func->args()) {
        arg.setName(fn.params[i].name);
        auto* alloca = create_entry_alloca(func, fn.params[i].name,
                                           param_types[i]);
        builder_.CreateStore(&arg, alloca);
        named_values_[fn.params[i].name] = alloca;
        i++;
    }

    auto& body = std::get<BlockExpr>(*fn.body);
    for (auto& stmt : body.statements) emit_stmt(stmt);

    if (body.tail_expr) {
        auto* val = emit_expr(*body.tail_expr);
        if (val) builder_.CreateRet(val);
    }

    auto* last_bb = builder_.GetInsertBlock();
    if (!last_bb->getTerminator()) {
        if (fn.return_type == AstType::Void) builder_.CreateRetVoid();
        else builder_.CreateRet(llvm::Constant::getNullValue(ret_type));
    }
}

void LLVMCodeGen::emit_stmt(const Stmt& stmt) {
    std::visit([this](auto& node) {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, LetStmt>) emit_let(node);
        else if constexpr (std::is_same_v<T, ReturnStmt>) emit_return(node);
        else if constexpr (std::is_same_v<T, ExprStmt>)
            emit_expr(*node.expr);
    }, stmt);
}

void LLVMCodeGen::emit_let(const LetStmt& stmt) {
    auto* val = emit_expr(*stmt.initializer);
    auto type = type_of(*stmt.initializer);
    auto* alloca = create_entry_alloca(
        builder_.GetInsertBlock()->getParent(), stmt.name, llvm_type(type));
    builder_.CreateStore(val, alloca);
    named_values_[stmt.name] = alloca;
}

void LLVMCodeGen::emit_return(const ReturnStmt& stmt) {
    if (stmt.value) {
        builder_.CreateRet(emit_expr(*stmt.value));
    } else {
        builder_.CreateRetVoid();
    }
}

llvm::Type* LLVMCodeGen::llvm_type(AstType type) {
    switch (type) {
        case AstType::Int: return llvm::Type::getInt64Ty(context_);
        case AstType::Float: return llvm::Type::getDoubleTy(context_);
        case AstType::Bool: return llvm::Type::getInt1Ty(context_);
        case AstType::String:
            return llvm::PointerType::get(context_, 0);
        case AstType::Void: return llvm::Type::getVoidTy(context_);
        default: return llvm::Type::getVoidTy(context_);
    }
}

AstType LLVMCodeGen::type_of(const Expr& expr) const {
    auto it = type_map_.find(static_cast<const void*>(&expr));
    return (it != type_map_.end()) ? it->second : AstType::Unknown;
}

llvm::AllocaInst* LLVMCodeGen::create_entry_alloca(
    llvm::Function* fn, const std::string& name, llvm::Type* type) {
    llvm::IRBuilder<> tmp(&fn->getEntryBlock(),
                          fn->getEntryBlock().begin());
    return tmp.CreateAlloca(type, nullptr, name);
}
