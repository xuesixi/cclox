//
// Created by Yue Xue  on 9/7/25.
//

#include "typechecker/expressions/as_expression.h"

#include "typechecker/ast.h"

TypePtr AsExpression::resolve_type(std::shared_ptr<ST_Scope> &scope) {
    auto t = expr->resolve_type(scope);
    line = expr->get_line();
    if (as_type->accept(t) || t->accept(as_type)) {
        // 断言的类型必须相关。可以 String | nil as String， 但不能 String as Animal
        return as_type;
    }
    throw mismatch(t, as_type, "cannot assert '{}' as '{}'");
}

void AsExpression::accept(AstCompiler &compiler) {
    compiler.visit_as_expr(this);
}

Expression::ExpressionType AsExpression::get_expression_type() const {
    return ExpressionType::As;
}
