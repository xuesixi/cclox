//
// Created by Yue Xue  on 9/3/25.
//

#ifndef CCLOX_EXPRESSION_H
#define CCLOX_EXPRESSION_H

#include "typechecker/tokenholder.h"
#include "typechecker/lox_type.h"
#include "typechecker/typeparser.h"

class AstCompiler;
class Expression;
class ST_Scope;

using ExprPtr = std::unique_ptr<Expression>;

class Expression {
public:

    enum class Assignability {
        None,
        Field,
        Variable,
    };

    enum class ExpressionType {
        And,
        As,
        Assignment,
        Binary,
        Call,
        Dot,
        Or,
        Primary,
        Unary
    };

    virtual ~Expression() {
    }

    /**
     * 检查并返回该表达式的返回值类型。
     * 于此同时，计算并储存行号。
     * 如果出现了问题，抛出异常
     * 该函数是幂等的。但是，除非特别情况，否则一般由表达式的 visit 内部调用。语句的 visit 不需要额外调用。
     * @return 该表达式的返回值类型
     */
    virtual TypePtr resolve_type(std::shared_ptr<ST_Scope> &scope) = 0;

    virtual void accept(AstCompiler &compiler) = 0;


    /**
     * 该表达式可否作为左值。
     */
    virtual Assignability get_assignability() const {
        return Assignability::None;
    }

    virtual ExpressionType get_expression_type() const = 0;

    int get_line() const {
        DEBUG_ASSERT(line != 0, "line num is 0, which means the line num is accessd before type resolution");
        return line;
    }

    int line = 0;
};


class ExpressionParser {
public:
    friend class StatementParser;

    explicit ExpressionParser(std::shared_ptr<TokenHolder> &th) : tokens(th) {
        type_parser = std::make_unique<TypeParser>(th);
    }

    ExprPtr parse_expression();

private:
    std::shared_ptr<TokenHolder> tokens;

    std::unique_ptr<TypeParser> type_parser;

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

    ExprPtr parse_as();

    ExprPtr parse_assignment();
};

#endif //CCLOX_EXPRESSION_H
