//
// Created by Yue Xue  on 9/10/25.
//

#include "typechecker/expressions/lambda_expression.h"

#include "objects/loxfunction.h"
#include "typechecker/types/function_type.h"

#include "typechecker/ast.h"
#include "typechecker/st_scope.h"

TypePtr LambdaExpression::resolve_type(std::shared_ptr<ST_Scope> &scope, TypePtr hint) {
    if (hint == nullptr) {
        throw MismatchedTypeError(fmt::format("line ?: the lambda expression type cannot be inferred"));
    }

    if (hint->type_enum != LoxTypeEnum::Function) {
        // todo: the line number does not exist before expr.resolve_type()
        throw MismatchedTypeError(fmt::format("line ?: expect a value of type '{}', but got a lambda expression", hint->to_string()));
    }
    const auto &fun_hint_type = std::static_pointer_cast<FunctionType>(hint);
    auto lambda_scope = std::make_shared<ST_Scope>(scope, fun_hint_type);
    lambda_scope->step_into(); // step out 由visit时执行。由于是函数, 不执行也行
    lambda_scope->function()->set_name("$(lambda)");
    const auto &params = lambda_scope->function()->get_type()->get_params();
    const size_t arg_count = params.size();
    for (size_t i = 1; i <= arg_count; i ++) {
        lambda_scope->add_local({fmt::format("@{}", i), TokenType::AT,  -1}, params.at(i-1));
        lambda_scope->initialize();
    }
    const auto &lambda_return_type = expr->resolve_type(lambda_scope);
    line = expr->line;
    if (fun_hint_type->get_return_type()->type_enum != LoxTypeEnum::Void && fun_hint_type->get_return_type()->accept(lambda_return_type) == false) {
        throw mismatch(fun_hint_type->get_return_type(), lambda_return_type);
    }
    this->scope = lambda_scope;

    return hint;
}

void LambdaExpression::accept(AstCompiler &compiler) {
    compiler.visit_lambda_expr(this);
}

Expression::ExpressionType LambdaExpression::get_expression_type() const {
    return ExpressionType::Lambda;
}
