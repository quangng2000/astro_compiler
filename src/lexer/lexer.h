#pragma once

#include "common/token.h"
#include <string>
#include <vector>

class Lexer {
public:
    explicit Lexer(std::string source);
    std::vector<Token> tokenize();

private:
    Token next_token();
    Token make_token(TokenKind kind, const std::string& text);
    Token read_number();
    Token read_string();
    Token read_identifier();
    Token read_punctuation();
    void skip_whitespace_and_comments();
    char peek() const;
    char peek_next() const;
    char advance();
    bool at_end() const;

    std::string source_;
    size_t pos_ = 0;
    int line_ = 1;
    int col_ = 1;
};
