#include "codegen.h"

CodeGen::CodeGen(TypeMap type_map) : type_map_(std::move(type_map)) {}

std::string CodeGen::generate(const Program& program) {
    emit_line("#include <stdio.h>");
    emit_line("#include <stdint.h>");
    emit_line("#include <stdbool.h>");
    emit_line("");

    // Forward declarations
    for (auto& fn : program.functions) {
        auto ret = fn.name == "main" ? "int" : c_type(fn.return_type);
        emit(ret + " " + fn.name + "(");
        for (size_t i = 0; i < fn.params.size(); ++i) {
            if (i > 0) emit(", ");
            emit(c_type(fn.params[i].type) + " " + fn.params[i].name);
        }
        emit_line(");");
    }
    emit_line("");

    for (auto& fn : program.functions) emit_function(fn);
    return out_.str();
}

void CodeGen::emit_function(const FnDecl& fn) {
    auto ret = fn.name == "main" ? "int" : c_type(fn.return_type);
    emit(ret + " " + fn.name + "(");
    for (size_t i = 0; i < fn.params.size(); ++i) {
        if (i > 0) emit(", ");
        emit(c_type(fn.params[i].type) + " " + fn.params[i].name);
    }
    emit_line(") {");
    indent();
    auto& body = std::get<BlockExpr>(*fn.body);
    for (auto& stmt : body.statements) emit_stmt(stmt);
    if (body.tail_expr) {
        emit("return ");
        emit_expr(*body.tail_expr);
        emit_line(";");
    }
    dedent();
    emit_line("}");
    emit_line("");
}

void CodeGen::emit_stmt(const Stmt& stmt) {
    std::visit([this](auto& node) {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, LetStmt>) emit_let(node);
        else if constexpr (std::is_same_v<T, ReturnStmt>) emit_return(node);
        else if constexpr (std::is_same_v<T, ExprStmt>) {
            emit(std::string(indent_level_ * 4, ' '));
            emit_expr(*node.expr);
            emit_line(";");
        }
    }, stmt);
}

void CodeGen::emit_let(const LetStmt& stmt) {
    auto type = type_of(*stmt.initializer);
    emit(std::string(indent_level_ * 4, ' '));
    emit("const " + c_type(type) + " " + stmt.name + " = ");
    emit_expr(*stmt.initializer);
    emit_line(";");
}

void CodeGen::emit_return(const ReturnStmt& stmt) {
    emit(std::string(indent_level_ * 4, ' '));
    if (stmt.value) {
        emit("return ");
        emit_expr(*stmt.value);
        emit_line(";");
    } else {
        emit_line("return;");
    }
}

void CodeGen::emit(const std::string& code) { out_ << code; }

void CodeGen::emit_line(const std::string& code) { out_ << code << "\n"; }

void CodeGen::indent() { indent_level_++; }

void CodeGen::dedent() { indent_level_--; }

std::string CodeGen::c_type(AstType type) const {
    switch (type) {
        case AstType::Int: return "int64_t";
        case AstType::Float: return "double";
        case AstType::Bool: return "bool";
        case AstType::String: return "const char*";
        case AstType::Void: return "void";
        default: return "void";
    }
}

AstType CodeGen::type_of(const Expr& expr) const {
    auto it = type_map_.find(static_cast<const void*>(&expr));
    return (it != type_map_.end()) ? it->second : AstType::Unknown;
}

std::string CodeGen::print_format(AstType type) const {
    switch (type) {
        case AstType::Int: return "%lld";
        case AstType::Float: return "%f";
        case AstType::Bool: return "%s";
        case AstType::String: return "%s";
        default: return "%d";
    }
}
