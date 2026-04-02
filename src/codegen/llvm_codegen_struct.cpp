#include "llvm_codegen.h"

void LLVMCodeGen::emit_impl_method(const std::string& target,
                                    const ImplMethod& m) {
    auto mangled = target + "_" + m.fn.name;
    auto* func = module_->getFunction(mangled);
    auto* entry = llvm::BasicBlock::Create(context_, "entry", func);
    builder_.SetInsertPoint(entry);
    named_values_.clear();

    size_t i = 0;
    for (auto& arg : func->args()) {
        auto& param = m.fn.params[i];
        arg.setName(param.name);
        auto* pty = func->getFunctionType()->getParamType(i);
        auto* alloca = create_entry_alloca(func, param.name, pty);
        builder_.CreateStore(&arg, alloca);
        named_values_[param.name] = alloca;
        i++;
    }

    auto& body = std::get<BlockExpr>(*m.fn.body);
    for (auto& stmt : body.statements) emit_stmt(stmt);
    if (body.tail_expr) {
        auto* val = emit_expr(*body.tail_expr);
        if (val) builder_.CreateRet(val);
    }
    auto* last_bb = builder_.GetInsertBlock();
    if (!last_bb->getTerminator()) {
        auto* retTy = llvm_type_ref(m.fn.return_type);
        if (resolve_type(m.fn.return_type) == AstType::Void)
            builder_.CreateRetVoid();
        else builder_.CreateRet(
            llvm::Constant::getNullValue(retTy));
    }
}

static llvm::AllocaInst* get_object_alloca(
    const Expr& expr,
    std::map<std::string, llvm::AllocaInst*>& named) {
    if (auto* ident = std::get_if<IdentExpr>(&expr))
        return named[ident->name];
    return nullptr;
}

llvm::Value* LLVMCodeGen::emit_struct_lit(const StructLitExpr& expr) {
    auto* st = get_struct_type(expr.struct_name);
    auto* alloca = builder_.CreateAlloca(st, nullptr, "tmp_struct");
    for (size_t i = 0; i < expr.fields.size(); ++i) {
        auto* val = emit_expr(*expr.fields[i].value);
        auto* gep = builder_.CreateStructGEP(st, alloca, i);
        builder_.CreateStore(val, gep);
    }
    return builder_.CreateLoad(st, alloca);
}

llvm::Value* LLVMCodeGen::emit_field_access(const FieldAccessExpr& expr) {
    auto sname = struct_name_of(*expr.object);
    auto* st = get_struct_type(sname);
    auto* info = registry_.lookup(sname);
    int idx = 0;
    for (auto& f : info->fields) {
        if (f.name == expr.field_name) break;
        idx++;
    }
    // Try to use the alloca directly for ident expressions
    auto* obj_alloca = get_object_alloca(*expr.object, named_values_);
    if (obj_alloca) {
        // For self (ptr to struct), load the ptr first
        if (obj_alloca->getAllocatedType()->isPointerTy()) {
            auto* ptr = builder_.CreateLoad(
                obj_alloca->getAllocatedType(), obj_alloca);
            auto* gep = builder_.CreateStructGEP(st, ptr, idx);
            return builder_.CreateLoad(st->getElementType(idx), gep);
        }
        auto* gep = builder_.CreateStructGEP(st, obj_alloca, idx);
        return builder_.CreateLoad(st->getElementType(idx), gep);
    }
    auto* val = emit_expr(*expr.object);
    auto* tmp = builder_.CreateAlloca(st, nullptr, "ftmp");
    builder_.CreateStore(val, tmp);
    auto* gep = builder_.CreateStructGEP(st, tmp, idx);
    return builder_.CreateLoad(st->getElementType(idx), gep);
}

llvm::Value* LLVMCodeGen::emit_method_call(const MethodCallExpr& expr) {
    auto sname = struct_name_of(*expr.object);
    auto mangled = sname + "_" + expr.method_name;
    auto* callee = module_->getFunction(mangled);
    auto* obj_alloca = get_object_alloca(*expr.object, named_values_);
    llvm::Value* self_ptr;
    if (obj_alloca) {
        if (obj_alloca->getAllocatedType()->isPointerTy())
            self_ptr = builder_.CreateLoad(
                obj_alloca->getAllocatedType(), obj_alloca);
        else
            self_ptr = obj_alloca;
    } else {
        auto* val = emit_expr(*expr.object);
        auto* st = get_struct_type(sname);
        auto* tmp = builder_.CreateAlloca(st, nullptr, "stmp");
        builder_.CreateStore(val, tmp);
        self_ptr = tmp;
    }
    std::vector<llvm::Value*> args = {self_ptr};
    for (auto& a : expr.args) args.push_back(emit_expr(*a));
    return builder_.CreateCall(callee, args);
}

llvm::Value* LLVMCodeGen::emit_static_call(const StaticCallExpr& expr) {
    auto mangled = expr.type_name + "_" + expr.method_name;
    auto* callee = module_->getFunction(mangled);
    std::vector<llvm::Value*> args;
    for (auto& a : expr.args) args.push_back(emit_expr(*a));
    return builder_.CreateCall(callee, args);
}
