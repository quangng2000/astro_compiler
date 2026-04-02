#pragma once

#include "source_location.h"
#include <string>

enum class TokenKind {
    // Literals
    IntLit, FloatLit, StringLit,

    // Identifiers & keywords
    Ident, Fn, Let, If, Else, Match, Return, True, False,

    // Type keywords
    IntType, FloatType, BoolType, StringType, VoidType,

    // Arithmetic operators
    Plus, Minus, Star, Slash, Percent,

    // Comparison operators
    Eq, NotEq, Lt, Gt, LtEq, GtEq,

    // Logical operators
    And, Or, Bang,

    // Punctuation
    LParen, RParen, LBrace, RBrace,
    Colon, Semicolon, Comma, Arrow, FatArrow, Assign,

    // Special
    Underscore, Eof,
};

struct Token {
    TokenKind kind;
    std::string text;
    SourceLocation loc;
};

inline std::string token_kind_name(TokenKind kind) {
    switch (kind) {
        case TokenKind::IntLit: return "IntLit";
        case TokenKind::FloatLit: return "FloatLit";
        case TokenKind::StringLit: return "StringLit";
        case TokenKind::Ident: return "Ident";
        case TokenKind::Fn: return "fn";
        case TokenKind::Let: return "let";
        case TokenKind::If: return "if";
        case TokenKind::Else: return "else";
        case TokenKind::Match: return "match";
        case TokenKind::Return: return "return";
        case TokenKind::True: return "true";
        case TokenKind::False: return "false";
        case TokenKind::IntType: return "int";
        case TokenKind::FloatType: return "float";
        case TokenKind::BoolType: return "bool";
        case TokenKind::StringType: return "string";
        case TokenKind::VoidType: return "void";
        case TokenKind::Plus: return "+";
        case TokenKind::Minus: return "-";
        case TokenKind::Star: return "*";
        case TokenKind::Slash: return "/";
        case TokenKind::Percent: return "%";
        case TokenKind::Eq: return "==";
        case TokenKind::NotEq: return "!=";
        case TokenKind::Lt: return "<";
        case TokenKind::Gt: return ">";
        case TokenKind::LtEq: return "<=";
        case TokenKind::GtEq: return ">=";
        case TokenKind::And: return "&&";
        case TokenKind::Or: return "||";
        case TokenKind::Bang: return "!";
        case TokenKind::LParen: return "(";
        case TokenKind::RParen: return ")";
        case TokenKind::LBrace: return "{";
        case TokenKind::RBrace: return "}";
        case TokenKind::Colon: return ":";
        case TokenKind::Semicolon: return ";";
        case TokenKind::Comma: return ",";
        case TokenKind::Arrow: return "->";
        case TokenKind::FatArrow: return "=>";
        case TokenKind::Assign: return "=";
        case TokenKind::Underscore: return "_";
        case TokenKind::Eof: return "EOF";
    }
    return "unknown";
}
