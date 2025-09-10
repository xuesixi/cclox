//
// Created by Yue Xue  on 9/3/25.
//

#include "typechecker/statement.h"

#include "typechecker/global_name_resolver.h"
#include "typechecker/statements/expression_statement.h"
#include "typechecker/statements/fun_statement.h"
#include "typechecker/statements/print_statement.h"
#include "typechecker/statements/var_statement.h"
#include "typechecker/statements/block_statement.h"
#include "typechecker/statements/if_statement.h"
#include "typechecker/statements/return_statement.h"
#include "typechecker/statements/while_statement.h"
#include "typechecker/types/function_type.h"
#include "typechecker/types/primitive_type.h"

TypePtr Statement::resolve_return_type() {
    return PrimitiveType::UnspecifiedType;
}

StmtPtr StatementParser::parse_statement(bool allow_declaration) {
    // todo
    while (tokens->match(TokenType::SEMICOLON)) {
        // 消耗无用的分号
    }
    if (tokens->match(TokenType::Print)) {
        return parse_print();
    }
    if (tokens->match(TokenType::LEFT_BRACE)) {
        return parse_block();
    }
    if (tokens->match(TokenType::Return)) {
        return parse_return();
    }
    if (tokens->match(TokenType::If)) {
        return parse_if();
    }
    if (tokens->match(TokenType::While)) {
        return parse_while();
    }
    if (allow_declaration) {
        if (tokens->match(TokenType::Var)) {
            return parse_var();
        }
        if (tokens->match(TokenType::Fun)) {
            return parse_fun();
        }
    }
    return parse_expression_statement();
}

StmtPtr StatementParser::parse_block() {
    std::vector<StmtPtr> body;
    depth++;
    while (tokens->match(TokenType::RIGHT_BRACE) == false) {
        body.push_back(parse_statement(true));
    }
    depth--;
    return std::make_unique<BlockStatement>(std::move(body));
}

StmtPtr StatementParser::parse_while() {
    auto condition = expr_parser->parse_expression();
    auto body = parse_statement(false);
    return std::make_unique<WhileStatement>(std::move(condition), std::move(body));
}

StmtPtr StatementParser::parse_print() {
    auto expr = expr_parser->parse_expression();
    tokens->consume();
    return std::make_unique<PrintStatement>(std::move(expr));
}

StmtPtr StatementParser::parse_var() {
    Token &name = tokens->consume(TokenType::IDENTIFIER, "expect an identifier as the variable name");
    if (depth == 0) {
        throw DefinitionPositionError(fmt::format("line {}: cannot use var to define variables in the global scope: {}",
                                                  name.get_line(), name.get_lexeme()));
    }
    TypePtr var_type;
    ExprPtr initializer;
    if (tokens->match(TokenType::COLON)) {
        // 有类型申明
        var_type = type_parser()->parse_type();
        if (tokens->match(TokenType::EQUAL)) {
            initializer = expr_parser->parse_expression();
        }
        tokens->consume();
        return std::make_unique<VarStatement>(name, var_type, std::move(initializer));
    } else {
        // 省略了类型申明
        var_type = PrimitiveType::UnspecifiedType;
        tokens->consume(TokenType::EQUAL, "a initializer is required if the type is not declared explicitly");
        initializer = expr_parser->parse_expression();
        tokens->consume();
        return std::make_unique<VarStatement>(name, var_type, std::move(initializer));
    }
}

StmtPtr StatementParser::parse_fun() {
    // fun hey(age: int, word: String) -> int {}

    Token &fun_name = tokens->consume(TokenType::IDENTIFIER, "expect an identifier as the function name");
    if (depth > 0) {
        throw DefinitionPositionError(fmt::format("line {}: can only define function in the global scope: {}",
                                                  fun_name.get_line(), fun_name.get_lexeme()));
    }

    // 解析参数列表
    tokens->consume(TokenType::LEFT_PAREN, "expect a '(' to start the function parameter list");
    std::vector<std::pair<Token, TypePtr> > parameters;
    if (tokens->match(TokenType::RIGHT_PAREN) == false) {
        do {
            Token &param_name = tokens->consume(TokenType::IDENTIFIER, "expect a parameter name here");
            tokens->consume(TokenType::COLON, "expect type annotation after each parameter");
            TypePtr param_type = type_parser()->parse_type();
            parameters.push_back({param_name, param_type});
        } while (tokens->match(TokenType::COMMA));
        tokens->consume(TokenType::RIGHT_PAREN, "expect a ')' to end the parameter list");
    }
    TypePtr return_type;
    if (tokens->match(TokenType::DASH_GREATER)) {
        return_type = type_parser()->parse_type();
    } else {
        return_type = PrimitiveType::VoidType; // 默认为 void
    }

    std::vector<TypePtr> type_params;
    type_params.reserve(parameters.size());
    for (auto &param: parameters) {
        type_params.push_back(param.second);
    }

    // todo: 函数类型不只是返回值，而需要包括参数列表
    auto fun_type = std::make_shared<FunctionType>(std::move(type_params), TypePtr(return_type));

    // 将这个函数添加到解析名字中
    name_resolver.declare_function(fun_name, fun_type);

    // 解析函数体
    tokens->consume(TokenType::LEFT_BRACE, "expect a '{' to start the function body");

    std::vector<StmtPtr> body;
    depth++;
    while (tokens->match(TokenType::RIGHT_BRACE) == false) {
        body.push_back(parse_statement(true));
    }

    depth--;
    return std::make_unique<FunStatement>(fun_name, std::move(parameters), fun_type, std::move(body));
}

StmtPtr StatementParser::parse_expression_statement() {
    auto expr = expr_parser->parse_expression();
    tokens->consume();
    return std::make_unique<ExpressionStatement>(std::move(expr));
}

StmtPtr StatementParser::parse_return() {
    if (tokens->match(TokenType::SEMICOLON)) {
        return std::make_unique<ReturnStatement>(nullptr, tokens->last_token().get_line());
    } else {
        auto stmt = std::make_unique<ReturnStatement>(expr_parser->parse_expression(), -1);
        tokens->consume();
        return stmt;
    }
}

StmtPtr StatementParser::parse_if() {
    auto condition = expr_parser->parse_expression();
    auto then_branch = parse_statement(false);
    StmtPtr else_branch = nullptr;
    if (tokens->match(TokenType::Else)) {
        else_branch = parse_statement(false);
    }
    return std::make_unique<IfStatement>(std::move(condition), std::move(then_branch), std::move(else_branch));
}
