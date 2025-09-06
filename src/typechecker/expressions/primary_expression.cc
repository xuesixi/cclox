//
// Created by Yue Xue  on 9/5/25.
//

#include "../../include/typechecker/expressions/primary_expression.h"

#include "typechecker/types/primitive_type.h"
#include "typechecker/ast.h"

TypePtr PrimaryExpression::resolve_type() {
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
        default:
            implementation_error("unknown primary expr type");
            return PrimitiveType::MismatchedType;
    }
}

void PrimaryExpression::accept(AstCompiler &compiler) {
    compiler.visit_primary_expr(this);
}
