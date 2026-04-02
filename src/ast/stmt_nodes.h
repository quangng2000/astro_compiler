#pragma once

#include "ast_fwd.h"
#include "type_nodes.h"
#include <string>
#include <vector>

struct LetStmt {
    std::string name;
    AstType declared_type = AstType::Unknown;
    ExprPtr initializer;
};

struct ReturnStmt {
    ExprPtr value;
};

struct ExprStmt {
    ExprPtr expr;
};

struct FnParam {
    std::string name;
    AstType type;
};

struct FnDecl {
    std::string name;
    std::vector<FnParam> params;
    AstType return_type = AstType::Void;
    ExprPtr body;
};

struct Program {
    std::vector<FnDecl> functions;
};
