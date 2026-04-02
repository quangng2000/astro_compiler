#include "lexer.h"
#include "common/error.h"
#include <unordered_map>

static const std::unordered_map<std::string, TokenKind> KEYWORDS = {
    {"fn", TokenKind::Fn},         {"let", TokenKind::Let},
    {"if", TokenKind::If},         {"else", TokenKind::Else},
    {"match", TokenKind::Match},   {"return", TokenKind::Return},
    {"true", TokenKind::True},     {"false", TokenKind::False},
    {"int", TokenKind::IntType},   {"float", TokenKind::FloatType},
    {"bool", TokenKind::BoolType}, {"string", TokenKind::StringType},
    {"void", TokenKind::VoidType}, {"struct", TokenKind::Struct},
    {"impl", TokenKind::Impl},     {"pub", TokenKind::Pub},
    {"self", TokenKind::SelfValue},
};

Lexer::Lexer(std::string source) : source_(std::move(source)) {}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (!at_end()) {
        skip_whitespace_and_comments();
        if (!at_end()) tokens.push_back(next_token());
    }
    tokens.push_back(make_token(TokenKind::Eof, ""));
    return tokens;
}

Token Lexer::next_token() {
    char c = peek();

    if (std::isdigit(c)) return read_number();
    if (c == '"') return read_string();
    if (std::isalpha(c) || c == '_') return read_identifier();

    return read_punctuation();
}

Token Lexer::read_number() {
    auto start = pos_;
    bool is_float = false;
    while (!at_end() && std::isdigit(peek())) advance();
    if (!at_end() && peek() == '.' && std::isdigit(peek_next())) {
        is_float = true;
        advance();
        while (!at_end() && std::isdigit(peek())) advance();
    }
    auto text = source_.substr(start, pos_ - start);
    auto kind = is_float ? TokenKind::FloatLit : TokenKind::IntLit;
    return make_token(kind, text);
}

Token Lexer::read_string() {
    auto loc = SourceLocation{line_, col_};
    advance(); // skip opening "
    std::string value;
    while (!at_end() && peek() != '"') {
        if (peek() == '\\') { advance(); value += peek(); }
        else { value += peek(); }
        advance();
    }
    if (at_end()) throw CompilerError("Lexer", "unterminated string", loc);
    advance(); // skip closing "
    return {TokenKind::StringLit, value, loc};
}

Token Lexer::read_identifier() {
    auto start = pos_;
    while (!at_end() && (std::isalnum(peek()) || peek() == '_')) advance();
    auto text = source_.substr(start, pos_ - start);
    if (text == "_") return make_token(TokenKind::Underscore, text);
    auto it = KEYWORDS.find(text);
    auto kind = (it != KEYWORDS.end()) ? it->second : TokenKind::Ident;
    return make_token(kind, text);
}
