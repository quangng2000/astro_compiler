#include "parser.h"

ExprPtr Parser::parse_expr() {
    return parse_expr_bp(0);
}

ExprPtr Parser::parse_expr_bp(int min_bp) {
    ExprPtr left;

    // Prefix operators
    if (check(TokenKind::Bang) || check(TokenKind::Minus)) {
        auto op = advance().kind;
        int rbp = prefix_bp(op);
        auto operand = parse_expr_bp(rbp);
        left = make_expr<UnaryExpr>(op, std::move(operand));
    } else {
        left = parse_primary();
    }

    // Infix operators
    while (is_infix_op(peek().kind)) {
        int lbp = infix_bp_left(peek().kind);
        if (lbp < min_bp) break;
        auto op = advance().kind;
        int rbp = infix_bp_right(op);
        auto right = parse_expr_bp(rbp);
        left = make_expr<BinaryExpr>(op, std::move(left), std::move(right));
    }
    return left;
}

ExprPtr Parser::parse_primary() {
    auto& tok = peek();

    switch (tok.kind) {
        case TokenKind::IntLit: {
            auto val = std::stoll(advance().text);
            return make_expr<IntLitExpr>(val);
        }
        case TokenKind::FloatLit: {
            auto val = std::stod(advance().text);
            return make_expr<FloatLitExpr>(val);
        }
        case TokenKind::StringLit: {
            auto val = advance().text;
            return make_expr<StringLitExpr>(std::move(val));
        }
        case TokenKind::True: advance(); return make_expr<BoolLitExpr>(true);
        case TokenKind::False: advance(); return make_expr<BoolLitExpr>(false);

        case TokenKind::Ident: {
            auto name = advance().text;
            if (check(TokenKind::LParen)) {
                auto args = parse_arg_list();
                return make_expr<CallExpr>(std::move(name), std::move(args));
            }
            return make_expr<IdentExpr>(std::move(name));
        }
        case TokenKind::If: return parse_if();
        case TokenKind::Match: return parse_match();
        case TokenKind::LBrace: return parse_block();
        case TokenKind::LParen: {
            advance();
            auto inner = parse_expr();
            expect(TokenKind::RParen);
            return inner;
        }
        default:
            error("unexpected token '" + tok.text + "'");
    }
}

std::vector<ExprPtr> Parser::parse_arg_list() {
    expect(TokenKind::LParen);
    std::vector<ExprPtr> args;
    while (!check(TokenKind::RParen)) {
        args.push_back(parse_expr());
        if (!check(TokenKind::RParen)) expect(TokenKind::Comma);
    }
    expect(TokenKind::RParen);
    return args;
}

ExprPtr Parser::parse_if() {
    expect(TokenKind::If);
    auto cond = parse_expr();
    auto then_branch = parse_block();
    expect(TokenKind::Else);
    auto else_branch = parse_block();
    return make_expr<IfExpr>(
        std::move(cond), std::move(then_branch), std::move(else_branch));
}

ExprPtr Parser::parse_match() {
    expect(TokenKind::Match);
    auto scrutinee = parse_expr();
    expect(TokenKind::LBrace);

    std::vector<MatchArm> arms;
    while (!check(TokenKind::RBrace)) {
        ExprPtr pattern;
        if (check(TokenKind::Underscore)) {
            advance();
            pattern = make_expr<IdentExpr>("_");
        } else {
            pattern = parse_primary();
        }
        expect(TokenKind::FatArrow);
        auto body = parse_expr();
        arms.push_back({std::move(pattern), std::move(body)});
        if (!check(TokenKind::RBrace)) expect(TokenKind::Comma);
    }
    expect(TokenKind::RBrace);
    return make_expr<MatchExpr>(std::move(scrutinee), std::move(arms));
}
