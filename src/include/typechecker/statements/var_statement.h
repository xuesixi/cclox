//
// Created by Yue Xue  on 9/5/25.
//

#ifndef CCLOX_VAR_STATEMENT_H
#define CCLOX_VAR_STATEMENT_H

#include "typechecker/statement.h"

class VarStatement: public Statement {
public:
    friend class AstCompiler;
    VarStatement(const Token &name, TypePtr &type, ExprPtr initializer): name(name), type(type), initializer(std::move(initializer)) {
        static_assert(!std::is_abstract_v<VarStatement>);
    }

    void accept(AstCompiler &compiler) override;

private:
    Token name;
    TypePtr type; // 如果为unspecified 则根据初始值自动推导
    ExprPtr initializer; // 为null则说明没有显式提供初始值，则会根据类型自动生成
};

#endif //CCLOX_VAR_STATEMENT_H