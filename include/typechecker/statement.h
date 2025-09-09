//
// Created by Yue Xue  on 9/3/25.
//

#ifndef CCLOX_STATEMENT_H
#define CCLOX_STATEMENT_H

#include "common.h"
#include "typechecker/tokenholder.h"
#include "typechecker/expression.h"
#include "typechecker/typeparser.h"

class AstCompiler;

class Statement {
public:
    virtual ~Statement() {
    }

    /**
     * 计算该语句的返回值。只有少部分有返回值的语句才需要重写该函数。
     * 例如说，函数的定义语句会判断自己标注的返回值和实际的返回值是否匹配。
     *  默认返回 Unspecified。如果出现错误，抛出异常
     */
    virtual TypePtr resolve_return_type();

    virtual void accept(AstCompiler &compiler) = 0;

};

using StmtPtr = std::unique_ptr<Statement>;

/**
 * 语句解析器，将 token 们解析为 ast。
 * 并不涉及任何字节码的生成。仅有少量的语义检查，包括：
 * - fun、class 只能写在最顶层。
 * - var 不能写在最顶层
 * - method, field 只能写在 class 内部
 * - 在全局层面，不会出现重复命名的fun、class
 */
class StatementParser {
public:

    explicit StatementParser(std::string && src): depth( ) {
        tokens = std::make_shared<TokenHolder>(std::move(src));
        expr_parser = std::make_unique<ExpressionParser>(tokens);
    }

    std::vector<StmtPtr> parse_all() {
        std::vector<StmtPtr> statements;
        while (tokens->match(TokenType::END_OF_FILE) == false) {
            statements.push_back(parse_statement());
        }
        return statements;
    }

    StmtPtr parse_statement();

    StmtPtr parse_block();

    /**
     * 在 print 已经被消费，next 为表达式的时候调用
     */
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

    /**
     * 在 return 已经被消费，next 为表达式的时候调用
     */
    StmtPtr parse_return();


private:

    std::unique_ptr<TypeParser> &type_parser() {
        return expr_parser->type_parser;
    }

    std::unique_ptr<ExpressionParser> expr_parser;
    std::shared_ptr<TokenHolder> tokens;
    int depth; // 作用域深度，用于判断当前是否处于全局作用域
    bool in_class = false; // 是否处于 class 内部
};


#endif //CCLOX_STATEMENT_H
