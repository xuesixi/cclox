//
// Created by Yue Xue  on 9/3/25.
//

#ifndef CCLOX_STATEMENT_H
#define CCLOX_STATEMENT_H

#include "tokenholder.h"
#include "typechecker/expression.h"
#include "typechecker/typeparser.h"

class Statement {
public:
    virtual ~Statement() {
    }

    virtual TypePtr resolve_return_type() {
        return PrimitiveType::UnspecifiedType;
    }
};

using StmtPtr = std::unique_ptr<Statement>;

/**
 * 语句解析器，将 token 们解析为 ast。
 * 并不涉及任何字节码的生成。仅有少量的语义检查，包括：
 * - fun、class 只能写在最顶层。
 * - var 不能写在最顶层
 * - method, field 只能写在 class 内部
 */
class StatementParser {
public:
    explicit StatementParser(std::shared_ptr<TokenHolder> &token_holder) : tokens(token_holder) {
        type_parser = std::make_unique<TypeParser>(token_holder);
        expr_parser = std::make_unique<ExpressionParser>(token_holder);
    }

    StmtPtr parse_statement();

    StmtPtr parse_print();

    /**
     * 在var已经被消费，next为变量名时调用
     */
    StmtPtr parse_var();

    /**
     * 在 fun 已经被消费，next 为函数名的时候调用
     */
    StmtPtr parse_fun();

    /**
     * next 为表达式的开头
     */
    StmtPtr parse_expression_statement();

private:
    std::unique_ptr<TypeParser> type_parser;
    std::unique_ptr<ExpressionParser> expr_parser;
    std::shared_ptr<TokenHolder> tokens;
    std::vector<StmtPtr> statements;
    int depth = 0; // 作用域深度，用于判断当前是否处于全局作用域
    bool in_class = false; // 是否处于 class 内部
};


#endif //CCLOX_STATEMENT_H
