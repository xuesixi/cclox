//
// Created by Yue Xue  on 9/3/25.
//

#ifndef CCLOX_EXPRESSION_H
#define CCLOX_EXPRESSION_H

#include "typechecker/tokenholder.h"
#include "typechecker/lox_type.h"

class AstCompiler;
class Expression;

using ExprPtr = std::unique_ptr<Expression>;

class Expression {
public:
    virtual ~Expression() {
    }

    /**
     * 检查并返回该表达式的返回值类型。
     * 于此同时，计算并储存行号。
     * 如果出现了问题，抛出异常
     * @return 该表达式的返回值类型
     */
    virtual TypePtr resolve_type() = 0;

    virtual void accept(AstCompiler &compiler) = 0;

    int get_line() const {
        return line;
    }

    int line = 0;
};


class ExpressionParser {
public:
    explicit ExpressionParser(std::shared_ptr<TokenHolder> &th) : tokens(th) {
    }

    ExprPtr parse_expression();

private:
    std::shared_ptr<TokenHolder> tokens;

    ExprPtr parse_primary();

    ExprPtr parse_call();

    ExprPtr parse_unary();

    ExprPtr parse_power();

    ExprPtr parse_factor();

    ExprPtr parse_term();

    ExprPtr parse_comparison();

    ExprPtr parse_equality();

    ExprPtr parse_and();

    ExprPtr parse_or();

    ExprPtr parse_assignment();
};

#endif //CCLOX_EXPRESSION_H
