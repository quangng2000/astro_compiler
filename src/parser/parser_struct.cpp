#include "parser.h"

StructDecl Parser::parse_struct() {
    expect(TokenKind::Struct);
    auto name = expect(TokenKind::Ident).text;
    expect(TokenKind::LBrace);

    std::vector<StructField> fields;
    while (!check(TokenKind::RBrace)) {
        bool is_pub = false;
        if (check(TokenKind::Pub)) { advance(); is_pub = true; }
        auto fname = expect(TokenKind::Ident).text;
        expect(TokenKind::Colon);
        auto ftype = parse_type_ref();
        fields.push_back({is_pub, fname, ftype});
        if (!check(TokenKind::RBrace)) expect(TokenKind::Comma);
    }
    expect(TokenKind::RBrace);
    return {name, std::move(fields)};
}

ImplBlock Parser::parse_impl() {
    expect(TokenKind::Impl);
    auto target = expect(TokenKind::Ident).text;
    expect(TokenKind::LBrace);

    std::vector<ImplMethod> methods;
    while (!check(TokenKind::RBrace)) {
        bool is_pub = false;
        if (check(TokenKind::Pub)) { advance(); is_pub = true; }

        auto fn = parse_function(is_pub);
        bool has_self = !fn.params.empty()
            && fn.params[0].name == "self";

        if (has_self) {
            fn.params[0].type = {AstType::Struct, target};
        }

        methods.push_back({is_pub, has_self, std::move(fn)});
    }
    expect(TokenKind::RBrace);
    return {target, std::move(methods)};
}
