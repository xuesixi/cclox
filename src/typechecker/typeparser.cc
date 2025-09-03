//
// Created by Yue Xue  on 9/3/25.
//

#include "../include/typechecker/typeparser.h"

std::unordered_map<std::string, size_t> LoxType::typename_to_id;
std::vector<std::string> LoxType::id_to_typename;

size_t LoxType::resolve_type_id(const std::string &name) {
    auto found = typename_to_id.find(name);
    if (found != typename_to_id.end()) {
        return found->second;
    }
    id_to_typename.push_back(name);
    size_t id = id_to_typename.size() - 1;
    typename_to_id.insert({name, id});
    return id;
}

std::string LoxType::read_typename_from_id(size_t type_id) {
    return id_to_typename.at(type_id);
}

TypePtr TypeParser::parse_type() {
    try {
        return parse_function();
    } catch (CompilerError &error) {
        std::cerr << "error: " << error.what();
        std::abort();
    }
}

TypePtr TypeParser::parse_function() {
    size_t save = next;
    if (match(TokenType::LEFT_PAREN)) {
        std::vector<LoxType::TypePtr> parameters;
        if (!match(TokenType::RIGHT_PAREN)) {
            do {
                parameters.push_back(parse_type());
            } while (match(TokenType::COMMA));
            consume(TokenType::RIGHT_PAREN, "expect a ')' for function");
        }
        if (!match(TokenType::DASH_GREATER)) {
            // 发现不是函数，于是返回save点
            next = save;
            return parse_union();
        }

        TypePtr return_type = parse_type();
        return std::make_shared<FunctionType>(std::move(parameters), std::move(return_type));

    } else {
        return parse_union();
    }
}


LoxType::TypePtr TypeParser::parse_union() {
    auto first = parse_intersection();
    if (match(TokenType::BAR)) {
        std::vector<TypePtr> unions;
        unions.push_back(first);
        do {
            unions.push_back(parse_intersection());
        } while (match(TokenType::BAR));
        return std::make_shared<UnionType>(std::move(unions));
    } else {
        return first;
    }
}

TypePtr TypeParser::parse_intersection() {
    auto first = parse_tuple();
    if (match(TokenType::AMPERSAND)) {
        std::vector<TypePtr> intersections;
        intersections.push_back(first);
        do {
            intersections.push_back(parse_tuple());
        } while (match(TokenType::AMPERSAND));
        return std::make_shared<IntersectionType>(std::move(intersections));
    } else {
        return first;
    }
}

TypePtr TypeParser::parse_tuple() {
    size_t save = next;
    if (match(TokenType::LEFT_PAREN)) {
        // (a, b, c)
        std::vector<TypePtr> tuple;
        if (match(TokenType::RIGHT_PAREN) == false) {
            do {
                tuple.push_back(parse_type());
            } while (match(TokenType::COMMA));
            consume(TokenType::RIGHT_PAREN, "expect a ')' for tuple");
        }

        if (tuple.size() > 1) {
            return std::make_shared<TupleType>(std::move(tuple));
        } else {
            next = save;
            return parse_array();
        }
    } else {
        return parse_array();
    }
}

TypePtr TypeParser::parse_array() {
    auto curr = parse_primary();
    while (match(TokenType::LEFT_BRACKET)) {
        consume(TokenType::RIGHT_BRACKET, "expect a ']' to declare an array");
        curr = std::make_shared<ArrayType>(std::move(curr));
    }
    return curr;
}

TypePtr TypeParser::parse_primary() {
    if (match(TokenType::LEFT_PAREN)) {
        auto t = parse_type();
        t->parenthesized = true;
        consume(TokenType::RIGHT_PAREN, "expect a ')' to end the primary");
        return t;
    } else {
        consume(TokenType::IDENTIFIER, "expect a identifier as type name here");
        Token &token = tokens.at(next - 1);
        auto primary = std::make_shared<PrimaryType>(token);
        return primary;
    }
}
