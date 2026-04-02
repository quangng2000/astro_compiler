#include "lexer.h"

Token Lexer::make_token(TokenKind kind, const std::string& text) {
    return {kind, text, {line_, col_}};
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
