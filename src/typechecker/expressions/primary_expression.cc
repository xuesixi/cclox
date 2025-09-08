//
// Created by Yue Xue  on 9/5/25.
//

#include "typechecker/expressions/primary_expression.h"
#include "scope.h"

#include "typechecker/types/primitive_type.h"
#include "typechecker/ast.h"

TypePtr PrimaryExpression::resolve_type(std::shared_ptr<Scope> &scope) {
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
            /**
             * 有多种可能：
             * - 本地变量
             * - todo: 外部变量
             * - 全局标识符
             */
            auto type = scope->resolve_local_type(value_token);
            if (type->type_enum != LoxTypeEnum::Unspecified) {
                return type;
            }
            return LoxType::find_name_type(value_token.get_lexeme());
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
Expression::Assignability PrimaryExpression::get_assignability() const {
    if (value_token.get_type() == TokenType::IDENTIFIER) {
        return Assignability::Variable;
    }
    return Assignability::None;
}
