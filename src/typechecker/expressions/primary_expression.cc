//
// Created by Yue Xue  on 9/5/25.
//

#include "typechecker/expressions/primary_expression.h"
#include "typechecker/st_scope.h"
#include "typechecker/global_name_resolver.h"
#include "scope.h"

#include "typechecker/types/primitive_type.h"
#include "typechecker/ast.h"

TypePtr PrimaryExpression::resolve_type(std::shared_ptr<ST_Scope> &scope, TypePtr hint) {
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
            type = scope->resolve_upvalue_type(value_token);
            if (type->type_enum != LoxTypeEnum::Unspecified) {
                return type;
            }
            type = name_resolver.resolve_name(value_token.get_lexeme()).first;
            if (type->type_enum == LoxTypeEnum::Unspecified) {
                throw VariableNotFoundError(fmt::format("line {}: the variable: '{}' is not defined", line, value_token.get_lexeme()));
            }
            return type;
        }
        default:
            ASSERT_UNREACHABLE();
    }
}

void PrimaryExpression::accept(AstCompiler &compiler) {
    compiler.visit_primary_expr(this);
}
