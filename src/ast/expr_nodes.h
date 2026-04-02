#pragma once

#include "ast_fwd.h"
#include "common/token.h"
#include <string>
#include <vector>

struct IntLitExpr {
    int64_t value;
};

struct FloatLitExpr {
    double value;
};

struct BoolLitExpr {
    bool value;
};

struct StringLitExpr {
    std::string value;
};

struct IdentExpr {
    std::string name;
};

struct BinaryExpr {
    TokenKind op;
    ExprPtr left;
    ExprPtr right;
};

struct UnaryExpr {
    TokenKind op;
    ExprPtr operand;
};

struct CallExpr {
    std::string callee;
    std::vector<ExprPtr> args;
};

struct IfExpr {
    ExprPtr condition;
    ExprPtr then_branch;
    ExprPtr else_branch;
};

struct MatchArm {
    ExprPtr pattern;
    ExprPtr body;
};

struct MatchExpr {
    ExprPtr scrutinee;
    std::vector<MatchArm> arms;
};

struct BlockExpr {
    std::vector<Stmt> statements;
    ExprPtr tail_expr;
};
