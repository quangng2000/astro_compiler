#pragma once

#include "common/token.h"
#include <string>
#include <unordered_map>

enum class AstType {
    Int, Float, Bool, String, Void, Unknown
};

inline AstType token_to_type(TokenKind kind) {
    static const std::unordered_map<TokenKind, AstType> map = {
        {TokenKind::IntType, AstType::Int},
        {TokenKind::FloatType, AstType::Float},
        {TokenKind::BoolType, AstType::Bool},
        {TokenKind::StringType, AstType::String},
        {TokenKind::VoidType, AstType::Void},
    };
    auto it = map.find(kind);
    return it != map.end() ? it->second : AstType::Unknown;
}

inline std::string type_to_string(AstType t) {
    switch (t) {
        case AstType::Int: return "int";
        case AstType::Float: return "float";
        case AstType::Bool: return "bool";
        case AstType::String: return "string";
        case AstType::Void: return "void";
        case AstType::Unknown: return "unknown";
    }
    return "unknown";
}
