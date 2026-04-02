#include "codegen.h"

void CodeGen::emit_expr(const Expr& expr) {
    std::visit([this, &expr](auto& node) {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, IntLitExpr>)
            emit(std::to_string(node.value) + "LL");
        else if constexpr (std::is_same_v<T, FloatLitExpr>)
            emit(std::to_string(node.value));
        else if constexpr (std::is_same_v<T, BoolLitExpr>)
            emit(node.value ? "true" : "false");
        else if constexpr (std::is_same_v<T, StringLitExpr>)
            emit("\"" + node.value + "\"");
        else if constexpr (std::is_same_v<T, IdentExpr>)
            emit(node.name);
        else if constexpr (std::is_same_v<T, BinaryExpr>)
            emit_binary(node);
        else if constexpr (std::is_same_v<T, UnaryExpr>) {
            emit(token_kind_name(node.op));
            emit("(");
            emit_expr(*node.operand);
            emit(")");
        }
        else if constexpr (std::is_same_v<T, CallExpr>)
            emit_call(node);
        else if constexpr (std::is_same_v<T, IfExpr>)
            emit_if(node);
        else if constexpr (std::is_same_v<T, MatchExpr>)
            emit_match(node);
        else if constexpr (std::is_same_v<T, BlockExpr>)
            emit_block(node, false);
    }, expr);
}

void CodeGen::emit_binary(const BinaryExpr& expr) {
    emit("(");
    emit_expr(*expr.left);
    emit(" " + token_kind_name(expr.op) + " ");
    emit_expr(*expr.right);
    emit(")");
}

void CodeGen::emit_call(const CallExpr& expr) {
    if (expr.callee == "print") {
        auto arg_type = type_of(*expr.args[0]);
        auto fmt = print_format(arg_type);
        emit("printf(\"" + fmt + "\\n\"");
        for (auto& arg : expr.args) {
            emit(", ");
            if (arg_type == AstType::Bool) {
                emit("(");
                emit_expr(*arg);
                emit(") ? \"true\" : \"false\"");
            } else {
                emit_expr(*arg);
            }
        }
        emit(")");
        return;
    }
    emit(expr.callee + "(");
    for (size_t i = 0; i < expr.args.size(); ++i) {
        if (i > 0) emit(", ");
        emit_expr(*expr.args[i]);
    }
    emit(")");
}

void CodeGen::emit_if(const IfExpr& expr) {
    emit("(");
    emit_expr(*expr.condition);
    emit(" ? ");
    emit_expr(*expr.then_branch);
    emit(" : ");
    emit_expr(*expr.else_branch);
    emit(")");
}

void CodeGen::emit_match(const MatchExpr& expr) {
    // Emit as nested ternaries
    for (size_t i = 0; i < expr.arms.size(); ++i) {
        auto& arm = expr.arms[i];
        bool is_wildcard = std::holds_alternative<IdentExpr>(*arm.pattern)
            && std::get<IdentExpr>(*arm.pattern).name == "_";
        if (is_wildcard) {
            emit_expr(*arm.body);
        } else {
            emit("(");
            emit_expr(*expr.scrutinee);
            emit(" == ");
            emit_expr(*arm.pattern);
            emit(" ? ");
            emit_expr(*arm.body);
            emit(" : ");
        }
    }
    // Close all non-wildcard parens
    for (auto& arm : expr.arms) {
        bool is_wildcard = std::holds_alternative<IdentExpr>(*arm.pattern)
            && std::get<IdentExpr>(*arm.pattern).name == "_";
        if (!is_wildcard) emit(")");
    }
}

void CodeGen::emit_block(const BlockExpr& block, bool is_top_level) {
    if (!is_top_level) emit("({");
    for (auto& stmt : block.statements) emit_stmt(stmt);
    if (block.tail_expr) emit_expr(*block.tail_expr);
    if (!is_top_level) emit(";})");
}
