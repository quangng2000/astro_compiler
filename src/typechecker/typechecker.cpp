#include "typechecker.h"
#include "common/error.h"

TypeMap TypeChecker::check(const Program& program) {
    // Register all function signatures first
    for (auto& fn : program.functions) {
        FnSignature sig;
        for (auto& p : fn.params) sig.param_types.push_back(p.type);
        sig.return_type = fn.return_type;
        globals_.define_fn(fn.name, sig);
    }
    // Built-in: print accepts any single arg
    globals_.define_fn("print", {{AstType::Unknown}, AstType::Void});

    for (auto& fn : program.functions) check_function(fn);
    return std::move(type_map_);
}

AstType TypeChecker::check_function(const FnDecl& fn) {
    TypeEnv local(&globals_);
    current_scope_ = &local;
    current_return_type_ = fn.return_type;
    for (auto& p : fn.params) local.define(p.name, p.type);
    auto body_type = check_expr(*fn.body);
    current_scope_ = &globals_;
    return body_type;
}

AstType TypeChecker::check_expr(const Expr& expr) {
    auto type = std::visit([this](auto& node) -> AstType {
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
            // Check arg count (skip for print)
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
        else if constexpr (std::is_same_v<T, BlockExpr>) {
            return check_block(node);
        }
        else return AstType::Unknown;
    }, expr);

    record(expr, type);
    return type;
}

AstType TypeChecker::check_block(const BlockExpr& block) {
    for (auto& stmt : block.statements) check_stmt(stmt);
    if (block.tail_expr) return check_expr(*block.tail_expr);
    return AstType::Void;
}

AstType TypeChecker::check_stmt(const Stmt& stmt) {
    return std::visit([this](auto& node) -> AstType {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, LetStmt>) {
            auto init_type = check_expr(*node.initializer);
            auto decl_type = node.declared_type;
            if (decl_type != AstType::Unknown)
                expect_type(decl_type, init_type, "let binding");
            else decl_type = init_type;
            current_scope_->define(node.name, decl_type);
            return AstType::Void;
        }
        else if constexpr (std::is_same_v<T, ReturnStmt>) {
            if (node.value) check_expr(*node.value);
            return AstType::Void;
        }
        else if constexpr (std::is_same_v<T, ExprStmt>) {
            check_expr(*node.expr);
            return AstType::Void;
        }
        else return AstType::Void;
    }, stmt);
}

void TypeChecker::expect_type(AstType expected, AstType actual,
                              const std::string& context) {
    if (expected != actual) {
        throw CompilerError("TypeCheck",
            "type mismatch in " + context + ": expected " +
            type_to_string(expected) + ", got " + type_to_string(actual));
    }
}

void TypeChecker::record(const Expr& expr, AstType type) {
    type_map_[static_cast<const void*>(&expr)] = type;
}
