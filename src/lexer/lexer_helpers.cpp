#include "lexer.h"
#include "common/error.h"

Token Lexer::make_token(TokenKind kind, const std::string& text) {
    return {kind, text, {line_, col_}};
}

void Lexer::skip_whitespace_and_comments() {
    while (!at_end()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') { advance(); continue; }
        if (c == '\n') { advance(); line_++; col_ = 1; continue; }
        if (c == '/' && peek_next() == '/') {
            while (!at_end() && peek() != '\n') advance();
            continue;
        }
        break;
    }
}

Token Lexer::read_punctuation() {
    auto loc = SourceLocation{line_, col_};
    char c = advance();
    switch (c) {
        case '(': return {TokenKind::LParen, "(", loc};
        case ')': return {TokenKind::RParen, ")", loc};
        case '{': return {TokenKind::LBrace, "{", loc};
        case '}': return {TokenKind::RBrace, "}", loc};
        case ':': return {TokenKind::Colon, ":", loc};
        case ';': return {TokenKind::Semicolon, ";", loc};
        case ',': return {TokenKind::Comma, ",", loc};
        case '+': return {TokenKind::Plus, "+", loc};
        case '*': return {TokenKind::Star, "*", loc};
        case '/': return {TokenKind::Slash, "/", loc};
        case '%': return {TokenKind::Percent, "%", loc};
        case '-':
            if (!at_end() && peek() == '>') {
                advance();
                return {TokenKind::Arrow, "->", loc};
            }
            return {TokenKind::Minus, "-", loc};
        case '=':
            if (!at_end() && peek() == '=') {
                advance();
                return {TokenKind::Eq, "==", loc};
            }
            if (!at_end() && peek() == '>') {
                advance();
                return {TokenKind::FatArrow, "=>", loc};
            }
            return {TokenKind::Assign, "=", loc};
        case '!':
            if (!at_end() && peek() == '=') {
                advance();
                return {TokenKind::NotEq, "!=", loc};
            }
            return {TokenKind::Bang, "!", loc};
        case '<':
            if (!at_end() && peek() == '=') {
                advance();
                return {TokenKind::LtEq, "<=", loc};
            }
            return {TokenKind::Lt, "<", loc};
        case '>':
            if (!at_end() && peek() == '=') {
                advance();
                return {TokenKind::GtEq, ">=", loc};
            }
            return {TokenKind::Gt, ">", loc};
        case '&':
            if (!at_end() && peek() == '&') {
                advance();
                return {TokenKind::And, "&&", loc};
            }
            break;
        case '|':
            if (!at_end() && peek() == '|') {
                advance();
                return {TokenKind::Or, "||", loc};
            }
            break;
        default: break;
    }
    throw CompilerError("Lexer", "unexpected character '" +
                        std::string(1, c) + "'", loc);
}

char Lexer::peek() const {
    return at_end() ? '\0' : source_[pos_];
}

char Lexer::peek_next() const {
    return (pos_ + 1 >= source_.size()) ? '\0' : source_[pos_ + 1];
}

char Lexer::advance() {
    char c = source_[pos_++];
    col_++;
    return c;
}

bool Lexer::at_end() const {
    return pos_ >= source_.size();
}
