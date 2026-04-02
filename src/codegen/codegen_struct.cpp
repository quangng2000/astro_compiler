#include "codegen.h"

void CodeGen::emit_struct_def(const StructDecl& decl) {
    emit("typedef struct { ");
    for (auto& f : decl.fields) {
        emit(c_type_ref(f.type) + " " + f.name + "; ");
    }
    emit_line("} " + decl.name + ";");
    emit_line("");
}

void CodeGen::emit_impl_methods(const ImplBlock& impl) {
    for (auto& m : impl.methods) {
        auto& fn = m.fn;
        auto ret = c_type_ref(fn.return_type);
        auto mangled = impl.target + "_" + fn.name;
        emit(ret + " " + mangled + "(");
        for (size_t i = 0; i < fn.params.size(); ++i) {
            if (i > 0) emit(", ");
            if (fn.params[i].name == "self") {
                emit(impl.target + "* self");
            } else {
                emit(c_type_ref(fn.params[i].type) + " " + fn.params[i].name);
            }
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
}

void CodeGen::emit_struct_lit(const StructLitExpr& expr) {
    emit("(" + expr.struct_name + "){");
    for (size_t i = 0; i < expr.fields.size(); ++i) {
        if (i > 0) emit(", ");
        emit("." + expr.fields[i].name + " = ");
        emit_expr(*expr.fields[i].value);
    }
    emit("}");
}

void CodeGen::emit_field_access(const FieldAccessExpr& expr) {
    bool is_ptr = std::holds_alternative<IdentExpr>(*expr.object)
        && std::get<IdentExpr>(*expr.object).name == "self";
    emit_expr(*expr.object);
    emit((is_ptr ? "->" : ".") + expr.field_name);
}

void CodeGen::emit_method_call(const MethodCallExpr& expr) {
    auto sname = struct_name_of(*expr.object);
    auto mangled = sname + "_" + expr.method_name;
    emit(mangled + "(&");
    emit_expr(*expr.object);
    for (auto& a : expr.args) { emit(", "); emit_expr(*a); }
    emit(")");
}

void CodeGen::emit_static_call(const StaticCallExpr& expr) {
    auto mangled = expr.type_name + "_" + expr.method_name;
    emit(mangled + "(");
    for (size_t i = 0; i < expr.args.size(); ++i) {
        if (i > 0) emit(", ");
        emit_expr(*expr.args[i]);
    }
    emit(")");
}
