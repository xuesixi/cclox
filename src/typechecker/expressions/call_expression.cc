//
// Created by Yue Xue  on 9/5/25.
//

#include "typechecker/expressions/call_expression.h"
#include "typechecker/types/function_type.h"

#include "typechecker/ast.h"

TypePtr CallExpression::resolve_type(std::shared_ptr<ST_Scope> &scope) {
    auto type = callee->resolve_type(scope);
    this->line = callee->get_line();
    if (type->type_enum != LoxTypeEnum::Function) {
        throw MismatchedTypeError(fmt::format("line {}: expect a callable but got '{}'", line, type->to_string()));
    }
    std::shared_ptr<FunctionType> callable = std::static_pointer_cast<FunctionType>(type);
    if (callable->parameters.size() != arguments.size()) {
        throw MismatchedTypeError(fmt::format("line {}: the callable expects '{}' arguments, but got '{}'",
                                              line,
                                              callable->parameters.size(), arguments.size()));
    }
    for (size_t i = 0; i < arguments.size(); i++) {
        auto arg_type = arguments.at(i)->resolve_type(scope);
        if (callable->parameters.at(i)->accept(arg_type) == false) {
            throw MismatchedTypeError(fmt::format("line {}: the {}th argument is expected to be '{}', but got '{}'",
                                                  arguments.at(i)->get_line(),
                                                  i + 1, callable->parameters.at(i)->to_string(),
                                                  arg_type->to_string()));
        }
    }
    return callable->return_type;
}

void CallExpression::accept(AstCompiler &compiler) {
    compiler.visit_call_expr(this);
}
