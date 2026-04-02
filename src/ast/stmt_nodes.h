#pragma once

#include "ast_fwd.h"
#include "type_nodes.h"
#include <string>
#include <vector>

struct LetStmt {
    std::string name;
    TypeRef declared_type;
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
    TypeRef type;
};

struct FnDecl {
    std::string name;
    std::vector<FnParam> params;
    TypeRef return_type = {AstType::Void, ""};
    ExprPtr body;
    bool is_pub = true;
};

struct StructField {
    bool is_pub = false;
    std::string name;
    TypeRef type;
};

struct StructDecl {
    std::string name;
    std::vector<StructField> fields;
};

struct ImplMethod {
    bool is_pub = false;
    bool has_self = false;
    FnDecl fn;
};

struct ImplBlock {
    std::string target;
    std::vector<ImplMethod> methods;
};

struct Program {
    std::vector<StructDecl> structs;
    std::vector<ImplBlock> impl_blocks;
    std::vector<FnDecl> functions;
};
