//
// Created by Yue Xue  on 9/5/25.
//

#include "typechecker/expressions/primary_expression.h"

#include "typechecker/types/primitive_type.h"
#include "typechecker/ast.h"

TypePtr PrimaryExpression::resolve_type() {
    this->line = value_token.get_line();
    switch (value_token.get_type()) {
        case TokenType::INTEGER:
            return PrimitiveType::IntType;
        case TokenType::FLOAT:
            return PrimitiveType::FloatType;
        case TokenType::True:
        case TokenType::False:
            return PrimitiveType::BoolType;
        case TokenType::STRING:
        case TokenType::FMT_STRING:
            return PrimitiveType::StringType;
        case TokenType::Nil:
            return PrimitiveType::NilType;
        case TokenType::IDENTIFIER: {
            return LoxType::find_class(value_token.get_lexeme());
        }
        default:
            ASSERT_UNREACHABLE();
    }
}

void PrimaryExpression::accept(AstCompiler &compiler) {
    compiler.visit_primary_expr(this);
}

/**
 * 如果是标识符，则可以被赋值
 */
bool PrimaryExpression::can_be_assign() const {
    if (value_token.get_type() == TokenType::IDENTIFIER) {
        return true;
    }
    return false;
}
