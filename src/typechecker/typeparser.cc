//
// Created by Yue Xue  on 9/3/25.
//

#include "../include/typechecker/typeparser.h"
#include "typechecker/types/array_type.h"
#include "typechecker/types/class_type.h"
#include "typechecker/types/function_type.h"
#include "typechecker/types/intersection_type.h"
#include "typechecker/types/primitive_type.h"
#include "typechecker/types/tuple_type.h"
#include "typechecker/types/union_type.h"

TypePtr PrimitiveType::IntType = std::make_shared<PrimitiveType>(LoxTypeEnum::Int);
TypePtr PrimitiveType::FloatType = std::make_shared<PrimitiveType>(LoxTypeEnum::Float);
TypePtr PrimitiveType::BoolType = std::make_shared<PrimitiveType>(LoxTypeEnum::Bool);
TypePtr PrimitiveType::AnyType  = std::make_shared<PrimitiveType>(LoxTypeEnum::Any);
TypePtr PrimitiveType::StringType = std::make_shared<PrimitiveType>(LoxTypeEnum::String);
TypePtr PrimitiveType::MismatchedType = std::make_shared<PrimitiveType>(LoxTypeEnum::Mismatched);
TypePtr PrimitiveType::UnspecifiedType = std::make_shared<PrimitiveType>(LoxTypeEnum::Unspecified);
TypePtr PrimitiveType::NilType = std::make_shared<PrimitiveType>(LoxTypeEnum::Nil);

std::unordered_map<std::string, TypePtr> LoxType::global_functions;
std::unordered_map<std::string, LoxType::ClassRecord> LoxType::global_classes;

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

bool LoxType::both_of_type(const TypePtr &a, const TypePtr &b, LoxTypeEnum type_enum) {
    return a->type_enum == type_enum && b->type_enum == type_enum;
}

bool LoxType::int_and_float(const TypePtr &a, const TypePtr &b) {
    return (a->type_enum == LoxTypeEnum::Int && b->type_enum == LoxTypeEnum::Float) || (a->type_enum == LoxTypeEnum::Float && b->type_enum == LoxTypeEnum::Int);
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
    size_t save = tokens->save_position();
    if (tokens->match(TokenType::LEFT_PAREN)) {
        std::vector<TypePtr> parameters;
        if (!tokens->match(TokenType::RIGHT_PAREN)) {
            do {
                parameters.push_back(parse_type());
            } while (tokens->match(TokenType::COMMA));
            tokens->consume(TokenType::RIGHT_PAREN, "expect a ')' for function");
        }
        if (!tokens->match(TokenType::DASH_GREATER)) {
            // 发现不是函数，于是返回save点
            tokens->rewind(save);
            return parse_union();
        }

        TypePtr return_type = parse_type();
        return std::make_shared<FunctionType>(std::move(parameters), std::move(return_type));
    } else {
        return parse_union();
    }
}


TypePtr TypeParser::parse_union() {
    auto first = parse_intersection();
    if (tokens->match(TokenType::BAR)) {
        std::vector<TypePtr> unions;
        unions.push_back(first);
        do {
            unions.push_back(parse_intersection());
        } while (tokens->match(TokenType::BAR));
        return std::make_shared<UnionType>(std::move(unions));
    } else {
        return first;
    }
}

TypePtr TypeParser::parse_intersection() {
    auto first = parse_tuple();
    if (tokens->match(TokenType::AMPERSAND)) {
        std::vector<TypePtr> intersections;
        intersections.push_back(first);
        do {
            intersections.push_back(parse_tuple());
        } while (tokens->match(TokenType::AMPERSAND));
        return std::make_shared<IntersectionType>(std::move(intersections));
    } else {
        return first;
    }
}

TypePtr TypeParser::parse_tuple() {
    size_t save = tokens->save_position();
    if (tokens->match(TokenType::LEFT_PAREN)) {
        // (a, b, c)
        std::vector<TypePtr> tuple;
        if (tokens->match(TokenType::RIGHT_PAREN) == false) {
            do {
                tuple.push_back(parse_type());
            } while (tokens->match(TokenType::COMMA));
            tokens->consume(TokenType::RIGHT_PAREN, "expect a ')' for tuple");
        }

        if (tuple.size() > 1) {
            return std::make_shared<TupleType>(std::move(tuple));
        } else {
            tokens->rewind(save);
            return parse_array();
        }
    } else {
        return parse_array();
    }
}

TypePtr TypeParser::parse_array() {
    auto curr = parse_primary();
    while (tokens->match(TokenType::LEFT_BRACKET)) {
        tokens->consume(TokenType::RIGHT_BRACKET, "expect a ']' to declare an array");
        curr = std::make_shared<ArrayType>(std::move(curr));
    }
    return curr;
}

TypePtr TypeParser::parse_primary() {
    if (tokens->match(TokenType::LEFT_PAREN)) {
        auto t = parse_type();
        t->parenthesized = true;
        tokens->consume(TokenType::RIGHT_PAREN, "expect a ')' to end the primary");
        return t;
    } else {
        Token &token = tokens->consume(TokenType::IDENTIFIER, "expect a identifier as type name here");
        if (token.get_lexeme() == "int") {
            return PrimitiveType::IntType;
        } else if (token.get_lexeme() == "bool") {
            return PrimitiveType::BoolType;
        }else if (token.get_lexeme() == "float") {
            return PrimitiveType::FloatType;
        }else if (token.get_lexeme() == "String") {
            return PrimitiveType::StringType;
        }else if (token.get_lexeme() == "any") {
            return PrimitiveType::AnyType;
        } else {
            return std::make_shared<ClassType>(token);
        }
    }
}
