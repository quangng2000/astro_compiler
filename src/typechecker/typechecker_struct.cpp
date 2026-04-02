#include "typechecker.h"
#include "common/error.h"

void TypeChecker::register_structs(const Program& program) {
    for (auto& s : program.structs) {
        StructInfo info{s.name, {}, {}};
        for (auto& f : s.fields)
            info.fields.push_back({f.name, f.type, f.is_pub});
        registry_.register_struct(s.name, std::move(info));
    }
}

void TypeChecker::register_impls(const Program& program) {
    for (auto& impl : program.impl_blocks) {
        auto* info = const_cast<StructInfo*>(registry_.lookup(impl.target));
        if (!info)
            throw CompilerError("TypeCheck",
                "impl for undefined struct '" + impl.target + "'");
        for (auto& m : impl.methods) {
            FnSignature sig;
            for (auto& p : m.fn.params)
                sig.param_types.push_back(resolve_type(p.type));
            sig.return_type = resolve_type(m.fn.return_type);
            info->methods.push_back(
                {m.fn.name, sig, m.is_pub, m.has_self});
            auto mangled = impl.target + "_" + m.fn.name;
            globals_.define_fn(mangled, sig);
        }
    }
}

AstType TypeChecker::check_struct_lit(const StructLitExpr& expr) {
    auto* info = registry_.lookup(expr.struct_name);
    if (!info)
        throw CompilerError("TypeCheck",
            "undefined struct '" + expr.struct_name + "'");
    for (auto& f : expr.fields) check_expr(*f.value);
    record_struct(reinterpret_cast<const Expr&>(expr), expr.struct_name);
    return AstType::Struct;
}

AstType TypeChecker::check_field_access(const FieldAccessExpr& expr) {
    check_expr(*expr.object);
    auto it = struct_names_.find(static_cast<const void*>(expr.object.get()));
    if (it == struct_names_.end())
        throw CompilerError("TypeCheck", "field access on non-struct");
    auto* field = registry_.lookup_field(it->second, expr.field_name);
    if (!field)
        throw CompilerError("TypeCheck",
            "no field '" + expr.field_name + "' on '" + it->second + "'");
    if (!field->is_pub && current_struct_ != it->second)
        throw CompilerError("TypeCheck",
            "field '" + expr.field_name + "' is private");
    return resolve_type(field->type);
}

AstType TypeChecker::check_method_call(const MethodCallExpr& expr) {
    check_expr(*expr.object);
    auto it = struct_names_.find(static_cast<const void*>(expr.object.get()));
    if (it == struct_names_.end())
        throw CompilerError("TypeCheck", "method call on non-struct");
    auto* method = registry_.lookup_method(it->second, expr.method_name);
    if (!method)
        throw CompilerError("TypeCheck",
            "no method '" + expr.method_name + "' on '" + it->second + "'");
    if (!method->is_pub && current_struct_ != it->second)
        throw CompilerError("TypeCheck",
            "method '" + expr.method_name + "' is private");
    for (auto& a : expr.args) check_expr(*a);
    return method->signature.return_type;
}

AstType TypeChecker::check_static_call(const StaticCallExpr& expr) {
    auto* method = registry_.lookup_method(expr.type_name, expr.method_name);
    if (!method)
        throw CompilerError("TypeCheck",
            "no method '" + expr.method_name + "' on '" + expr.type_name + "'");
    if (!method->is_pub && current_struct_ != expr.type_name)
        throw CompilerError("TypeCheck",
            "method '" + expr.method_name + "' is private");
    for (auto& a : expr.args) check_expr(*a);
    if (method->signature.return_type == AstType::Struct)
        record_struct(reinterpret_cast<const Expr&>(expr), expr.type_name);
    return method->signature.return_type;
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
            auto decl_type = resolve_type(node.declared_type);
            if (decl_type != AstType::Unknown)
                expect_type(decl_type, init_type, "let binding");
            else decl_type = init_type;
            current_scope_->define(node.name, decl_type);
            if (decl_type == AstType::Struct) {
                auto it = struct_names_.find(
                    static_cast<const void*>(node.initializer.get()));
                if (it != struct_names_.end())
                    var_struct_names_[node.name] = it->second;
                else if (node.declared_type.is_struct())
                    var_struct_names_[node.name] = node.declared_type.name;
            }
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

void TypeChecker::record_struct(const Expr& expr, const std::string& name) {
    struct_names_[static_cast<const void*>(&expr)] = name;
}

AstType TypeChecker::resolve_type(const TypeRef& ref) const {
    return ref.base;
}
