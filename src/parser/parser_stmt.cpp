#include "parser.h"
#include "common/error.h"

Parser::Parser(std::vector<Token> tokens)
    : tokens_(std::move(tokens)) {}

Program Parser::parse() {
    Program program;
    while (!at_end()) {
        if (check(TokenKind::Struct)) {
            program.structs.push_back(parse_struct());
        } else if (check(TokenKind::Impl)) {
            program.impl_blocks.push_back(parse_impl());
        } else {
            program.functions.push_back(parse_function());
        }
    }
    return program;
}

FnDecl Parser::parse_function(bool is_pub) {
    expect(TokenKind::Fn);
    auto name = expect(TokenKind::Ident).text;
    expect(TokenKind::LParen);

    std::vector<FnParam> params;
    while (!check(TokenKind::RParen)) {
        if (check(TokenKind::SelfValue)) {
            advance();
            params.push_back({"self", {AstType::Struct, ""}});
        } else {
            auto pname = expect(TokenKind::Ident).text;
            expect(TokenKind::Colon);
            params.push_back({pname, parse_type_ref()});
        }
        if (!check(TokenKind::RParen)) expect(TokenKind::Comma);
    }
    expect(TokenKind::RParen);

    TypeRef ret_type = {AstType::Void, ""};
    if (check(TokenKind::Arrow)) {
        advance();
        ret_type = parse_type_ref();
    }

    auto body = parse_block();
    return {name, std::move(params), ret_type, std::move(body), is_pub};
}

TypeRef Parser::parse_type_ref() {
    auto tok = advance();
    auto t = token_to_type(tok.kind);
    if (t != AstType::Unknown) return {t, ""};
    if (tok.kind == TokenKind::Ident) return {AstType::Struct, tok.text};
    error("expected type, got '" + tok.text + "'");
}

Stmt Parser::parse_statement() {
    if (check(TokenKind::Let)) return parse_let();
    if (check(TokenKind::Return)) return parse_return();
    auto expr = parse_expr();
    expect(TokenKind::Semicolon);
    return ExprStmt{std::move(expr)};
}

LetStmt Parser::parse_let() {
    expect(TokenKind::Let);
    auto name = expect(TokenKind::Ident).text;
    TypeRef type = {AstType::Unknown, ""};
    if (check(TokenKind::Colon)) {
        advance();
        type = parse_type_ref();
    }
    expect(TokenKind::Assign);
    auto init = parse_expr();
    expect(TokenKind::Semicolon);
    return {name, type, std::move(init)};
}

ReturnStmt Parser::parse_return() {
    expect(TokenKind::Return);
    if (check(TokenKind::Semicolon)) { advance(); return {nullptr}; }
    auto value = parse_expr();
    expect(TokenKind::Semicolon);
    return {std::move(value)};
}

// Utilities
const Token& Parser::peek() const { return tokens_[pos_]; }

const Token& Parser::peek_at(size_t offset) const {
    return tokens_[pos_ + offset];
}

Token Parser::advance() { return tokens_[pos_++]; }

Token Parser::expect(TokenKind kind) {
    if (peek().kind != kind) {
        error("expected '" + token_kind_name(kind) +
              "', got '" + peek().text + "'");
    }
    return advance();
}

bool Parser::check(TokenKind kind) const {
    return peek().kind == kind;
}

bool Parser::at_end() const {
    return peek().kind == TokenKind::Eof;
}

void Parser::error(const std::string& msg) {
    throw CompilerError("Parser", msg, peek().loc);
}
