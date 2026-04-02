#pragma once

#include <memory>
#include <variant>
#include <vector>

struct IntLitExpr;
struct FloatLitExpr;
struct BoolLitExpr;
struct StringLitExpr;
struct IdentExpr;
struct BinaryExpr;
struct UnaryExpr;
struct CallExpr;
struct IfExpr;
struct MatchExpr;
struct BlockExpr;
struct StructLitExpr;
struct FieldAccessExpr;
struct MethodCallExpr;
struct StaticCallExpr;

using Expr = std::variant<
    IntLitExpr, FloatLitExpr, BoolLitExpr, StringLitExpr,
    IdentExpr, BinaryExpr, UnaryExpr, CallExpr,
    IfExpr, MatchExpr, BlockExpr,
    StructLitExpr, FieldAccessExpr, MethodCallExpr, StaticCallExpr
>;

using ExprPtr = std::unique_ptr<Expr>;

struct LetStmt;
struct ReturnStmt;
struct ExprStmt;

using Stmt = std::variant<LetStmt, ReturnStmt, ExprStmt>;

template<typename T, typename... Args>
ExprPtr make_expr(Args&&... args) {
    return std::make_unique<Expr>(T{std::forward<Args>(args)...});
}
