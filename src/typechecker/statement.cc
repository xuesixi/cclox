//
// Created by Yue Xue  on 9/3/25.
//

#include "../include/typechecker/statement.h"

#include "typechecker/statements/expression_statement.h"
#include "typechecker/statements/fun_statement.h"
#include "typechecker/statements/print_statement.h"
#include "typechecker/statements/var_statement.h"

StmtPtr StatementParser::parse_statement() {
    // todo
    if (tokens->match(TokenType::Print)) {
        return parse_print();
    }
    if (tokens->match(TokenType::Var)) {
        return parse_var();
    }
    if (tokens->match(TokenType::Fun)) {
        return parse_fun();
    }

    return parse_expression_statement();

}

StmtPtr StatementParser::parse_print() {
    auto expr = expr_parser->parse_expression();
    return std::make_unique<PrintStatement>(std::move(expr));
}

StmtPtr StatementParser::parse_var() {
    if (depth == 0) {
        throw DefinitionPositionError("cannot use var to define variables in the global scope");
    }
    Token &name = tokens->consume(TokenType::IDENTIFIER, "expect an identifier as the variable name");
    TypePtr var_type;
    ExprPtr initializer;
    if (tokens->match(TokenType::COLON)) {
        // 有类型申明
        var_type = type_parser->parse_type();
        if (tokens->match(TokenType::EQUAL)) {
            initializer = expr_parser->parse_expression();
        }
        return std::make_unique<VarStatement>(name, var_type, std::move(initializer));
    } else {
        // 省略了类型申明
        var_type = PrimitiveType::UnspecifiedType;
        tokens->consume(TokenType::EQUAL, "a initializer is required if the type is not declared explicitly");
        initializer = expr_parser->parse_expression();
        return std::make_unique<VarStatement>(name, var_type, std::move(initializer));
    }
}

StmtPtr StatementParser::parse_fun() {
    // fun hey(age: int, word: String) -> int {}
    Token &fun_name = tokens->consume(TokenType::IDENTIFIER, "expect an identifier as the function name");
    tokens->consume(TokenType::LEFT_PAREN, "expect a '(' to start the function parameter list");
    std::vector<std::pair<Token, TypePtr> > parameters;
    if (tokens->match(TokenType::RIGHT_PAREN) == false) {
        do {
            Token &param_name = tokens->consume(TokenType::IDENTIFIER, "expect a parameter name here");
            tokens->consume(TokenType::COLON, "expect type annotation after each parameter");
            TypePtr param_type = type_parser->parse_type();
            parameters.push_back({param_name, param_type});
        } while (tokens->match(TokenType::COMMA));
    }
    TypePtr return_type;
    if (tokens->match(TokenType::DASH_GREATER)) {
        return_type = type_parser->parse_type();
    } else {
        return_type = PrimitiveType::UnspecifiedType; // 默认为 void
    }
    tokens->consume(TokenType::LEFT_BRACE, "expect a '{' to start the function body");

    std::vector<StmtPtr> body;
    depth++;
    while (tokens->match(TokenType::RIGHT_BRACE) == false) {
        body.push_back(parse_statement());
    }

    depth--;
    return std::make_unique<FunStatement>(fun_name, std::move(parameters), return_type, std::move(body));
}

StmtPtr StatementParser::parse_expression_statement() {
    auto expr = expr_parser->parse_expression();
    return std::make_unique<ExpressionStatement>(std::move(expr));
}
