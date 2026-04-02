#include "typechecker.h"
#include "common/error.h"

TypeCheckResult TypeChecker::check(const Program& program) {
    register_structs(program);
    register_impls(program);

    globals_.define_fn("print", {{AstType::Unknown}, AstType::Void});
    for (auto& fn : program.functions) {
        FnSignature sig;
        for (auto& p : fn.params) sig.param_types.push_back(resolve_type(p.type));
        sig.return_type = resolve_type(fn.return_type);
        globals_.define_fn(fn.name, sig);
    }

    for (auto& fn : program.functions) check_function(fn);
    for (auto& impl : program.impl_blocks) {
        current_struct_ = impl.target;
        for (auto& m : impl.methods) check_function(m.fn);
        current_struct_ = "";
    }
    return {std::move(type_map_), std::move(struct_names_),
            std::move(registry_)};
}

AstType TypeChecker::check_function(const FnDecl& fn) {
    TypeEnv local(&globals_);
    current_scope_ = &local;
    current_return_type_ = resolve_type(fn.return_type);
    for (auto& p : fn.params) {
        local.define(p.name, resolve_type(p.type));
        if (p.type.is_struct())
            var_struct_names_[p.name] = p.type.name;
    }
    auto body_type = check_expr(*fn.body);
    current_scope_ = &globals_;
    return body_type;
}

AstType TypeChecker::check_expr(const Expr& expr) {
    auto type = std::visit([this, &expr](auto& node) -> AstType {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, IntLitExpr>) return AstType::Int;
        else if constexpr (std::is_same_v<T, FloatLitExpr>) return AstType::Float;
        else if constexpr (std::is_same_v<T, BoolLitExpr>) return AstType::Bool;
        else if constexpr (std::is_same_v<T, StringLitExpr>) return AstType::String;
        else if constexpr (std::is_same_v<T, IdentExpr>) {
            auto t = current_scope_->lookup(node.name);
            if (t == AstType::Unknown)
                throw CompilerError("TypeCheck",
                    "undefined variable '" + node.name + "'");
            if (t == AstType::Struct) {
                auto vit = var_struct_names_.find(node.name);
                if (vit != var_struct_names_.end())
                    struct_names_[static_cast<const void*>(&expr)] =
                        vit->second;
            }
            return t;
        }
        else if constexpr (std::is_same_v<T, BinaryExpr>) {
            auto lt = check_expr(*node.left);
            auto rt = check_expr(*node.right);
            expect_type(lt, rt, "binary operator");
            if (node.op == TokenKind::Eq || node.op == TokenKind::NotEq ||
                node.op == TokenKind::Lt || node.op == TokenKind::Gt ||
                node.op == TokenKind::LtEq || node.op == TokenKind::GtEq ||
                node.op == TokenKind::And || node.op == TokenKind::Or)
                return AstType::Bool;
            return lt;
        }
        else if constexpr (std::is_same_v<T, UnaryExpr>) {
            auto ot = check_expr(*node.operand);
            if (node.op == TokenKind::Bang) return AstType::Bool;
            return ot;
        }
        else if constexpr (std::is_same_v<T, CallExpr>) {
            auto* sig = current_scope_->lookup_fn(node.callee);
            if (!sig) sig = globals_.lookup_fn(node.callee);
            if (!sig)
                throw CompilerError("TypeCheck",
                    "undefined function '" + node.callee + "'");
            if (node.callee != "print" &&
                node.args.size() != sig->param_types.size())
                throw CompilerError("TypeCheck",
                    "wrong number of arguments for '" + node.callee + "'");
            for (size_t i = 0; i < node.args.size(); ++i)
                check_expr(*node.args[i]);
            return sig->return_type;
        }
        else if constexpr (std::is_same_v<T, IfExpr>) {
            check_expr(*node.condition);
            auto tt = check_expr(*node.then_branch);
            auto et = check_expr(*node.else_branch);
            expect_type(tt, et, "if/else branches");
            return tt;
        }
        else if constexpr (std::is_same_v<T, MatchExpr>) {
            check_expr(*node.scrutinee);
            AstType arm_type = AstType::Unknown;
            for (auto& arm : node.arms) {
                auto bt = check_expr(*arm.body);
                if (arm_type == AstType::Unknown) arm_type = bt;
                else expect_type(arm_type, bt, "match arms");
            }
            return arm_type;
        }
        else if constexpr (std::is_same_v<T, BlockExpr>)
            return check_block(node);
        else if constexpr (std::is_same_v<T, StructLitExpr>)
            return check_struct_lit(node);
        else if constexpr (std::is_same_v<T, FieldAccessExpr>)
            return check_field_access(node);
        else if constexpr (std::is_same_v<T, MethodCallExpr>)
            return check_method_call(node);
        else if constexpr (std::is_same_v<T, StaticCallExpr>)
            return check_static_call(node);
        else return AstType::Unknown;
    }, expr);

    record(expr, type);
    return type;
}
