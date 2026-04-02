#include "llvm_codegen.h"
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>

void LLVMCodeGen::write_ir(const std::string& path) {
    std::error_code ec;
    llvm::raw_fd_ostream os(path, ec, llvm::sys::fs::OF_None);
    module_->print(os, nullptr);
    os.flush();
}

int LLVMCodeGen::compile_to_object(const std::string& path) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    auto triple_str = llvm::sys::getDefaultTargetTriple();
    llvm::Triple triple(triple_str);
    module_->setTargetTriple(triple);

    std::string error;
    auto* target = llvm::TargetRegistry::lookupTarget(triple, error);
    if (!target) { llvm::errs() << error; return 1; }

    auto* machine = target->createTargetMachine(
        triple, "generic", "", llvm::TargetOptions(),
        std::nullopt);
    module_->setDataLayout(machine->createDataLayout());

    std::error_code ec;
    llvm::raw_fd_ostream os(path, ec, llvm::sys::fs::OF_None);
    if (ec) { llvm::errs() << ec.message(); return 1; }

    llvm::legacy::PassManager pm;
    if (machine->addPassesToEmitFile(
            pm, os, nullptr, llvm::CodeGenFileType::ObjectFile)) {
        llvm::errs() << "target cannot emit object file\n";
        return 1;
    }
    pm.run(*module_);
    os.flush();
    return 0;
}

int LLVMCodeGen::compile_to_binary(const std::string& obj_path,
                                    const std::string& out_path) {
    auto cmd = "cc -o " + out_path + " " + obj_path + " 2>&1";
    return std::system(cmd.c_str());
}

llvm::Function* LLVMCodeGen::get_or_declare_printf() {
    if (auto* f = module_->getFunction("printf")) return f;
    auto* i8ptr = llvm::PointerType::get(context_, 0);
    auto* ft = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(context_), {i8ptr}, true);
    return llvm::Function::Create(
        ft, llvm::Function::ExternalLinkage, "printf", module_.get());
}

llvm::Value* LLVMCodeGen::emit_print_call(const CallExpr& expr) {
    auto* printf_fn = get_or_declare_printf();
    auto arg_type = type_of(*expr.args[0]);
    auto* val = emit_expr(*expr.args[0]);

    std::string fmt;
    switch (arg_type) {
        case AstType::Int:   fmt = "%lld\n"; break;
        case AstType::Float: fmt = "%f\n"; break;
        default:             fmt = "%s\n"; break;
    }
    auto* fmt_str = builder_.CreateGlobalString(fmt);

    if (arg_type == AstType::Bool) {
        auto* t = builder_.CreateGlobalString("true");
        auto* f = builder_.CreateGlobalString("false");
        val = builder_.CreateSelect(val, t, f);
    }
    return builder_.CreateCall(printf_fn, {fmt_str, val});
}

llvm::Value* LLVMCodeGen::emit_match(const MatchExpr& expr) {
    auto* scrutinee = emit_expr(*expr.scrutinee);
    auto* fn = builder_.GetInsertBlock()->getParent();
    auto* merge_bb = llvm::BasicBlock::Create(context_, "match.end", fn);

    std::vector<std::pair<llvm::Value*, llvm::BasicBlock*>> results;

    for (size_t i = 0; i < expr.arms.size(); ++i) {
        auto& arm = expr.arms[i];
        bool is_wildcard = std::holds_alternative<IdentExpr>(*arm.pattern)
            && std::get<IdentExpr>(*arm.pattern).name == "_";

        if (is_wildcard) {
            auto* val = emit_expr(*arm.body);
            results.push_back({val, builder_.GetInsertBlock()});
            builder_.CreateBr(merge_bb);
        } else {
            auto* pat = emit_expr(*arm.pattern);
            auto* cmp = builder_.CreateICmpEQ(scrutinee, pat);
            auto* then_bb = llvm::BasicBlock::Create(context_, "match.arm", fn);
            auto* next_bb = llvm::BasicBlock::Create(context_, "match.next", fn);
            builder_.CreateCondBr(cmp, then_bb, next_bb);

            builder_.SetInsertPoint(then_bb);
            auto* val = emit_expr(*arm.body);
            results.push_back({val, builder_.GetInsertBlock()});
            builder_.CreateBr(merge_bb);

            builder_.SetInsertPoint(next_bb);
        }
    }

    builder_.SetInsertPoint(merge_bb);
    if (results.empty()) return nullptr;
    auto* phi = builder_.CreatePHI(results[0].first->getType(), results.size());
    for (auto& [val, bb] : results) phi->addIncoming(val, bb);
    return phi;
}

llvm::Value* LLVMCodeGen::emit_block(const BlockExpr& block) {
    for (auto& stmt : block.statements) emit_stmt(stmt);
    if (block.tail_expr) return emit_expr(*block.tail_expr);
    return nullptr;
}
