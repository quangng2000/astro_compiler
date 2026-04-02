#include "parser.h"

ExprPtr Parser::parse_block() {
    expect(TokenKind::LBrace);
    std::vector<Stmt> stmts;
    ExprPtr tail = nullptr;

    while (!check(TokenKind::RBrace)) {
        // Try to detect if this is a tail expression (no semicolon)
        if (check(TokenKind::Let) || check(TokenKind::Return)) {
            stmts.push_back(parse_statement());
        } else {
            auto expr = parse_expr();
            if (check(TokenKind::Semicolon)) {
                advance();
                stmts.push_back(ExprStmt{std::move(expr)});
            } else {
                tail = std::move(expr);
                break;
            }
        }
    }
    expect(TokenKind::RBrace);
    return make_expr<BlockExpr>(std::move(stmts), std::move(tail));
}

bool Parser::is_infix_op(TokenKind kind) {
    return infix_bp_left(kind) > 0;
}

int Parser::infix_bp_left(TokenKind kind) {
    switch (kind) {
        case TokenKind::Or:    return 2;
        case TokenKind::And:   return 4;
        case TokenKind::Eq:
        case TokenKind::NotEq: return 6;
        case TokenKind::Lt:
        case TokenKind::Gt:
        case TokenKind::LtEq:
        case TokenKind::GtEq:  return 8;
        case TokenKind::Plus:
        case TokenKind::Minus: return 10;
        case TokenKind::Star:
        case TokenKind::Slash:
        case TokenKind::Percent: return 12;
        default: return 0;
    }
}

int Parser::infix_bp_right(TokenKind kind) {
    return infix_bp_left(kind) + 1;
}

int Parser::prefix_bp(TokenKind kind) {
    switch (kind) {
        case TokenKind::Bang:
        case TokenKind::Minus: return 14;
        default: return 0;
    }
}
