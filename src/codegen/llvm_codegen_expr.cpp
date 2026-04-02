#include "llvm_codegen.h"

llvm::Value* LLVMCodeGen::emit_expr(const Expr& expr) {
    return std::visit([this](auto& node) -> llvm::Value* {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, IntLitExpr>)
            return llvm::ConstantInt::get(
                llvm::Type::getInt64Ty(context_), node.value, true);
        else if constexpr (std::is_same_v<T, FloatLitExpr>)
            return llvm::ConstantFP::get(
                llvm::Type::getDoubleTy(context_), node.value);
        else if constexpr (std::is_same_v<T, BoolLitExpr>)
            return llvm::ConstantInt::get(
                llvm::Type::getInt1Ty(context_), node.value ? 1 : 0);
        else if constexpr (std::is_same_v<T, StringLitExpr>)
            return builder_.CreateGlobalString(node.value);
        else if constexpr (std::is_same_v<T, IdentExpr>) {
            auto* alloca = named_values_[node.name];
            return builder_.CreateLoad(alloca->getAllocatedType(),
                                       alloca, node.name);
        }
        else if constexpr (std::is_same_v<T, BinaryExpr>)
            return emit_binary(node);
        else if constexpr (std::is_same_v<T, UnaryExpr>)
            return emit_unary(node);
        else if constexpr (std::is_same_v<T, CallExpr>)
            return emit_call(node);
        else if constexpr (std::is_same_v<T, IfExpr>)
            return emit_if(node);
        else if constexpr (std::is_same_v<T, MatchExpr>)
            return emit_match(node);
        else if constexpr (std::is_same_v<T, BlockExpr>)
            return emit_block(node);
        else return nullptr;
    }, expr);
}

llvm::Value* LLVMCodeGen::emit_binary(const BinaryExpr& expr) {
    auto* l = emit_expr(*expr.left);
    auto* r = emit_expr(*expr.right);
    bool is_float = l->getType()->isDoubleTy();

    switch (expr.op) {
        case TokenKind::Plus:  return is_float ? builder_.CreateFAdd(l, r) : builder_.CreateAdd(l, r);
        case TokenKind::Minus: return is_float ? builder_.CreateFSub(l, r) : builder_.CreateSub(l, r);
        case TokenKind::Star:  return is_float ? builder_.CreateFMul(l, r) : builder_.CreateMul(l, r);
        case TokenKind::Slash: return is_float ? builder_.CreateFDiv(l, r) : builder_.CreateSDiv(l, r);
        case TokenKind::Percent: return builder_.CreateSRem(l, r);
        case TokenKind::Eq:    return is_float ? builder_.CreateFCmpOEQ(l, r) : builder_.CreateICmpEQ(l, r);
        case TokenKind::NotEq: return is_float ? builder_.CreateFCmpONE(l, r) : builder_.CreateICmpNE(l, r);
        case TokenKind::Lt:    return is_float ? builder_.CreateFCmpOLT(l, r) : builder_.CreateICmpSLT(l, r);
        case TokenKind::Gt:    return is_float ? builder_.CreateFCmpOGT(l, r) : builder_.CreateICmpSGT(l, r);
        case TokenKind::LtEq:  return is_float ? builder_.CreateFCmpOLE(l, r) : builder_.CreateICmpSLE(l, r);
        case TokenKind::GtEq:  return is_float ? builder_.CreateFCmpOGE(l, r) : builder_.CreateICmpSGE(l, r);
        case TokenKind::And:   return builder_.CreateAnd(l, r);
        case TokenKind::Or:    return builder_.CreateOr(l, r);
        default: return nullptr;
    }
}

llvm::Value* LLVMCodeGen::emit_unary(const UnaryExpr& expr) {
    auto* val = emit_expr(*expr.operand);
    if (expr.op == TokenKind::Minus)
        return val->getType()->isDoubleTy()
            ? builder_.CreateFNeg(val) : builder_.CreateNeg(val);
    return builder_.CreateNot(val);
}

llvm::Value* LLVMCodeGen::emit_call(const CallExpr& expr) {
    if (expr.callee == "print") return emit_print_call(expr);
    auto* callee = module_->getFunction(expr.callee);
    std::vector<llvm::Value*> args;
    for (auto& a : expr.args) args.push_back(emit_expr(*a));
    return builder_.CreateCall(callee, args);
}

llvm::Value* LLVMCodeGen::emit_if(const IfExpr& expr) {
    auto* cond = emit_expr(*expr.condition);
    auto* fn = builder_.GetInsertBlock()->getParent();
    auto* then_bb = llvm::BasicBlock::Create(context_, "then", fn);
    auto* else_bb = llvm::BasicBlock::Create(context_, "else", fn);
    auto* merge_bb = llvm::BasicBlock::Create(context_, "merge", fn);
    builder_.CreateCondBr(cond, then_bb, else_bb);

    builder_.SetInsertPoint(then_bb);
    auto* then_val = emit_expr(*expr.then_branch);
    builder_.CreateBr(merge_bb);
    then_bb = builder_.GetInsertBlock();

    builder_.SetInsertPoint(else_bb);
    auto* else_val = emit_expr(*expr.else_branch);
    builder_.CreateBr(merge_bb);
    else_bb = builder_.GetInsertBlock();

    builder_.SetInsertPoint(merge_bb);
    auto* phi = builder_.CreatePHI(then_val->getType(), 2);
    phi->addIncoming(then_val, then_bb);
    phi->addIncoming(else_val, else_bb);
    return phi;
}
