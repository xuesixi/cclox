//
// Created by Yue Xue  on 9/5/25.
//

#include "typechecker/expressions/dot_expression.h"
#include "typechecker/global_name_resolver.h"

#include "typechecker/ast.h"
#include "typechecker/types/class_type.h"

TypePtr DotExpression::resolve_type(std::shared_ptr<ST_Scope> &scope) {
    auto target_type = target->resolve_type(scope);
    this->line = target->get_line();
    if (target_type->type_enum != LoxTypeEnum::Class) {
        throw MismatchedTypeError(fmt::format("cannot access member of type {}", target_type->to_string()));
    }
    auto instance_class = std::static_pointer_cast<ClassType>(target_type);
    return name_resolver.find_class_member(instance_class->get_type_id(), target_field.get_lexeme());
}

Expression::Assignability DotExpression::get_assignability() const {
    return Assignability::Field;
}

void DotExpression::accept(AstCompiler &compiler) {
    compiler.visit_dot_expr(this);
}
