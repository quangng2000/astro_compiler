#include "parser.h"

ExprPtr Parser::parse_struct_literal(std::string name) {
    expect(TokenKind::LBrace);
    std::vector<StructLitField> fields;
    while (!check(TokenKind::RBrace)) {
        auto fname = expect(TokenKind::Ident).text;
        expect(TokenKind::Colon);
        auto value = parse_expr();
        fields.push_back({fname, std::move(value)});
        if (!check(TokenKind::RBrace)) expect(TokenKind::Comma);
    }
    expect(TokenKind::RBrace);
    return make_expr<StructLitExpr>(std::move(name), std::move(fields));
}

ExprPtr Parser::apply_postfix(ExprPtr left) {
    while (check(TokenKind::Dot)) {
        advance();
        auto member = expect(TokenKind::Ident).text;
        if (check(TokenKind::LParen)) {
            auto args = parse_arg_list();
            left = make_expr<MethodCallExpr>(
                std::move(left), std::move(member), std::move(args));
        } else {
            left = make_expr<FieldAccessExpr>(
                std::move(left), std::move(member));
        }
    }
    return left;
}

ExprPtr Parser::parse_static_call(std::string type_name) {
    expect(TokenKind::ColonColon);
    auto method = expect(TokenKind::Ident).text;
    auto args = parse_arg_list();
    return make_expr<StaticCallExpr>(
        std::move(type_name), std::move(method), std::move(args));
}
