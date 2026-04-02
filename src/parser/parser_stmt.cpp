#include "parser.h"
#include "common/error.h"

Parser::Parser(std::vector<Token> tokens)
    : tokens_(std::move(tokens)) {}

Program Parser::parse() {
    Program program;
    while (!at_end()) {
        program.functions.push_back(parse_function());
    }
    return program;
}

FnDecl Parser::parse_function() {
    expect(TokenKind::Fn);
    auto name = expect(TokenKind::Ident).text;
    expect(TokenKind::LParen);

    std::vector<FnParam> params;
    while (!check(TokenKind::RParen)) {
        auto pname = expect(TokenKind::Ident).text;
        expect(TokenKind::Colon);
        auto ptype = parse_type();
        params.push_back({pname, ptype});
        if (!check(TokenKind::RParen)) expect(TokenKind::Comma);
    }
    expect(TokenKind::RParen);

    AstType ret_type = AstType::Void;
    if (check(TokenKind::Arrow)) {
        advance();
        ret_type = parse_type();
    }

    auto body = parse_block();
    return {name, std::move(params), ret_type, std::move(body)};
}

AstType Parser::parse_type() {
    auto tok = advance();
    auto t = token_to_type(tok.kind);
    if (t == AstType::Unknown) {
        error("expected type, got '" + tok.text + "'");
    }
    return t;
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

    AstType type = AstType::Unknown;
    if (check(TokenKind::Colon)) {
        advance();
        type = parse_type();
    }

    expect(TokenKind::Assign);
    auto init = parse_expr();
    expect(TokenKind::Semicolon);
    return {name, type, std::move(init)};
}

ReturnStmt Parser::parse_return() {
    expect(TokenKind::Return);
    if (check(TokenKind::Semicolon)) {
        advance();
        return {nullptr};
    }
    auto value = parse_expr();
    expect(TokenKind::Semicolon);
    return {std::move(value)};
}

// Utility methods
const Token& Parser::peek() const { return tokens_[pos_]; }

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
