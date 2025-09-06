//
// Created by Yue Xue  on 9/5/25.
//

#ifndef CCLOX_BLOCK_STATEMENT_H
#define CCLOX_BLOCK_STATEMENT_H

#include "typechecker/statement.h"

class BlockStatement : public Statement {
public:
    friend class AstCompiler;
    explicit BlockStatement(std::vector<StmtPtr> &&body) : body(std::move(body)) {
        static_assert(!std::is_abstract_v<BlockStatement>);
    }

    void accept(AstCompiler &compiler) override;

private:
    std::vector<StmtPtr> body;

public:

    /**
     * 按顺序遍历，如果有任何一个语句返回，那么本语句的返回值与之相同。如果都没有，也是 unspecified
     */
    TypePtr resolve_return_type() override {
        for (auto &statement: body) {
            auto statement_return_type = statement->resolve_return_type();
            if (statement_return_type->type_enum != LoxTypeEnum::Unspecified) {
                return statement_return_type;
            }
        }
        return PrimitiveType::UnspecifiedType;
    }
};

#endif //CCLOX_BLOCK_STATEMENT_H
