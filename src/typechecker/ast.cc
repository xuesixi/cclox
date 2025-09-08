//
// Created by Yue Xue  on 9/5/25.
//
#include "typechecker/ast.h"
#include "typechecker/st_scope.h"
#include "typechecker/expressions/unary_expression.h"
#include "typechecker/st_runtime.h"
#include "chunk.h"
#include "scope.h"
#include "typechecker/global_name_resolver.h"
#include "typechecker/expressions/and_expression.h"
#include "typechecker/expressions/assignment_expression.h"
#include "typechecker/expressions/binary_expression.h"
#include "typechecker/expressions/or_expression.h"
#include "typechecker/expressions/primary_expression.h"
#include "typechecker/statements/fun_statement.h"
#include "typechecker/statements/print_statement.h"
#include "typechecker/statements/var_statement.h"
#include "typechecker/types/union_type.h"

std::shared_ptr<LoxFunction> AstCompiler::compile(std::string &&source) {
    StatementParser parser(std::move(source));
    statements = parser.parse_all();
    return nullptr;
}

void AstCompiler::visit_fun_statement(FunStatement *fun) {
    scope = std::make_shared<ST_Scope>(nullptr);
    scope->step_into();

    for (auto & parameter : fun->parameters) {
        // 将每一个参数作为本地变量处理
        scope->add_local(parameter.first, parameter.second);
    }
    bool accepted_return = false;

    // 处理函数体内的每个语句
    for (auto & stmt : fun->body) {

        visit_statement(stmt);

        // 如果该函数有标注返回值，那么需要检查每个语句是否成功返回了合适的返回值
        // 在第一个成功返回处，停止代码生成
        if (fun->return_type->type_enum != LoxTypeEnum::Unspecified) {
            const auto &stmt_type = stmt->resolve_return_type();
            if (stmt_type->type_enum != LoxTypeEnum::Unspecified) {
                // 有返回值！
                if (fun->return_type->accept(stmt_type)) {
                    accepted_return = true;
                    break;
                } else {
                    // 如果返回值类型不匹配，则抛出异常。
                    // 实际上我觉得这个检查时不必要的，因为 return 语句本身会根据 scope 来检查返回值的类型是否合适
                    throw MismatchedTypeError(fmt::format("the function expects return type of {}, but got {}", fun->return_type->to_string(), stmt_type->to_string()));
                }
            }
        }
    }

    if (fun->return_type->type_enum != LoxTypeEnum::Unspecified && accepted_return == false) {
        // 如果有标注返回值，但函数体内没有返回，则抛出异常
        throw MismatchedTypeError(fmt::format("not all control flows return the expected type: {}", fun->return_type->to_string()));
    }
    uint8_t clear_amount = scope->step_out(0);
    current_chunk().write_opcode(Opcode::ClearN, 0);
    current_chunk().write_operand(clear_amount, 0);

    Runtime::record_allocation(scope->function_); // todo: 考虑 st_runtime

    auto [type, index] = name_resolver.resolve_name(fun->fun_name.get_lexeme());

    st_runtime.define_global(index, scope->function_);

    scope = std::move(scope->outer_);
}

void AstCompiler::visit_var_statement(VarStatement *stmt) {

    scope->add_local(stmt->name);

    if (stmt->type->type_enum == LoxTypeEnum::Unspecified) {
        DEBUG_ASSERT(stmt->initializer != nullptr, "both initializer and type are empty");
        // 如果没有指明类型，则类型由初始值决定
        auto type = stmt->initializer->resolve_type(scope);
        visit_expression(stmt->initializer); // 将初始值入栈
    } else {

        // 如果指明了类型，那么初始值可能为空

        if (stmt->initializer != nullptr) {
            // 初始值不为空, 判断指定的类型是否和初始值的实际类型匹配
            auto type = stmt->initializer->resolve_type(scope);
            if (stmt->type->accept(type)) {
                visit_expression(stmt->initializer);
            } else {
                throw MismatchedTypeError(fmt::format("annotated type: {} does not match initializer type: {}", stmt->type->to_string(), type->to_string()));
            }
        } else {
            // 初始值为空，此时，初始值由指定的类型决定。但也有些类型不允许初始值
            int line = stmt->name.get_line();
            switch (stmt->type->type_enum) {
                case LoxTypeEnum::Int: {
                    auto imme_index = Chunk::to_immediate(0);
                    current_chunk().write_opcode(Opcode::LoadImmediate, line);
                    current_chunk().write_operand(imme_index.value(), line);
                    break;
                }
                case LoxTypeEnum::Float: {
                    auto imme_index = Chunk::to_immediate(0.0);
                    current_chunk().write_opcode(Opcode::LoadImmediate, line);
                    current_chunk().write_operand(imme_index.value(), line);
                    break;
                }
                case LoxTypeEnum::Bool: {
                    current_chunk().write_opcode(Opcode::LoadFalse, line);
                    break;
                }
                case LoxTypeEnum::String: {
                    current_chunk().write_opcode(Opcode::LoadEmptyString, line);
                    break;
                }
                case LoxTypeEnum::Nil:
                case LoxTypeEnum::Any: {
                    current_chunk().write_opcode(Opcode::LoadNil, line);
                    break;
                }
                case LoxTypeEnum::Union: {
                    auto u = std::static_pointer_cast<UnionType>(stmt->type);
                    if (u->contains_nil()) {
                        current_chunk().write_opcode(Opcode::LoadNil, line);
                    } else {
                        throw NoInitializationError(fmt::format("the annotated type: {} must be initialized", stmt->type->to_string()));
                    }
                    break;
                }
                default:
                    throw NoInitializationError(fmt::format("the annotated type: {} must be initialized", stmt->type->to_string()));

            }
        }
    }
}

void AstCompiler::visit_print_statement(PrintStatement *stmt) {
    stmt->expr->resolve_type(scope);
    visit_expression(stmt->expr);
    emit_opcode(Opcode::Print, stmt->expr->get_line());
}

void AstCompiler::visit_and_expr(AndExpression *expr) {
    expr->resolve_type(scope);
    visit_expression(expr->left);
    auto short_circuit = emit_jump(Opcode::JumpIfFalse, 0);
    visit_expression(expr->right);
    patch_jump(short_circuit);
}

void AstCompiler::visit_as_expr(AsExpression *expr) {
}

void AstCompiler::visit_assignment_expr(AssignmentExpression *expr) {
    expr->resolve_type(scope);
    auto target_type = expr->left->get_expression_type();
    if (target_type == Expression::ExpressionType::Primary) {
        auto left = std::unique_ptr<PrimaryExpression>(static_cast<PrimaryExpression *>(expr->left.release()));
        if (left->value_token.get_type() != TokenType::IDENTIFIER) {
            throw InvalidAssignmentTargetError(fmt::format("invalid assignment target"));
        }
        Token &target_name = left->value_token;
        auto local_found = scope->resolve_local(target_name);
        if (local_found.has_value() == false) {
            throw VariableNotFoundError(fmt::format("no such variable: {}", target_name.get_lexeme()));
        }
        emit_opcode(Opcode::SetLocal, left->get_line());
        emit_operand_1(local_found.value(), left->get_line());
    } else {
        NOT_IMPLEMENTED();
    }
}

void AstCompiler::visit_binary_expr(BinaryExpression *expr) {
    expr->resolve_type(scope);
    visit_expression(expr->left);
    visit_expression(expr->right);
    int line = expr->op.get_line();
    switch (expr->op.get_type()) {
        case TokenType::PLUS:
            emit_opcode(Opcode::Add, line);
            break;
        case TokenType::MINUS:
            emit_opcode(Opcode::Subtract, line);
            break;
        case TokenType::STAR:
            emit_opcode(Opcode::Multipy, line);
            break;
        case TokenType::SLASH:
            emit_opcode(Opcode::Divide, line);
            break;
        case TokenType::STAR_STAR:
            emit_opcode(Opcode::Power, line);
            break;
        case TokenType::EQUAL_EQUAL:
            emit_opcode(Opcode::Equal, line);
            break;
        case TokenType::BANG_EQUAL:
            emit_opcode(Opcode::Equal, line);
            emit_opcode(Opcode::Not, line);
            break;
        case TokenType::LESS:
            emit_opcode(Opcode::Less, line);
            break;
        case TokenType::LESS_EQUAL:
            emit_opcode(Opcode::Greater, line);
            emit_opcode(Opcode::Not, line);
            break;
        case TokenType::GREATER:
            emit_opcode(Opcode::Greater, line);
            break;
        case TokenType::GREATER_EQUAL:
            emit_opcode(Opcode::Less, line);
            emit_opcode(Opcode::Not, line);
            break;
        default:
            ASSERT_UNREACHABLE();
    }
}

void AstCompiler::visit_or_expr(OrExpression *expr) {
    expr->resolve_type(scope);
    visit_expression(expr->left);
    auto short_circuit = emit_jump(Opcode::JumpIfTrue, 0);
    visit_expression(expr->right);
    patch_jump(short_circuit);
}

void AstCompiler::visit_unary_expr(UnaryExpression *expr) {
    auto type = expr->resolve_type(scope);
    visit_expression(expr->operand);
    if (expr->op.get_type() == TokenType::MINUS) {
        emit_opcode(Opcode::Negate, expr->get_line());
    } else if (expr->op.get_type() == TokenType::BANG) {
        emit_opcode(Opcode::Not, expr->get_line());
    } else {
        ASSERT_UNREACHABLE();
    }
}

Chunk & AstCompiler::current_chunk() {
    return scope->function_->get_chunk();
}

void AstCompiler::emit_opcode(Opcode op_code, int line) {
    current_chunk().write_opcode(op_code, line);
}

void AstCompiler::emit_operand_flexible(size_t operand, int line) {
    DEBUG_ASSERT(within<uint16_t>(operand), "size overflow");
    current_chunk().write_operand(operand, line);
}

void AstCompiler::emit_operand_1(size_t operand, int line) {
    DEBUG_ASSERT(within<uint8_t>(operand), "size overflow");
    emit_operand_flexible(operand, line);
}

void AstCompiler::emit_operand_2(size_t operand, int line) {
    DEBUG_ASSERT(within<uint16_t>(operand), "operand is not within uint16");
    current_chunk().write_operand_2(operand, line);
}

OperandSize AstCompiler::emit_jump(Opcode jump_operation, int line) {
    current_chunk().write_opcode(jump_operation, line);
    current_chunk().write_operand_2(0, line);
    return current_chunk().code_size() - 2;
}

void AstCompiler::patch_jump(OperandSize from_label) {
    size_t distance = current_chunk().code_size() - from_label - 2;
    if (!within<uint16_t>(distance)) {
        throw JumpDistanceOverflowError("the distance to jump is too much to be encoded as an uint16");
    }
    auto [high, low] = u16_to_u8(distance);
    current_chunk().code_at(from_label) = low;
    current_chunk().code_at(from_label + 1) = high;
}

void AstCompiler::loop_back(size_t destination) {
    DEBUG_ASSERT(current_chunk().code_size() >= destination, "loop back is jumping forward!");
    size_t distance = current_chunk().code_size() - destination + 3;

    if (!within<uint16_t>(distance)) {
        throw JumpDistanceOverflowError("the distance to jump is too much to be encoded as an uint16");
    }
    emit_opcode(Opcode::JumpBack, 0);
    emit_operand_2(distance, 0);
}

bool AstCompiler::is_global_scope() const {
    return scope == nullptr;
}
