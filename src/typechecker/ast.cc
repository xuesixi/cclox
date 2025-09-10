//
// Created by Yue Xue  on 9/5/25.
//
#include "typechecker/ast.h"
#include "typechecker/st_scope.h"
#include "typechecker/expressions/unary_expression.h"
#include "typechecker/st_runtime.h"
#include "chunk.h"
#include "disassembler.h"
#include "objects/loxclosure.h"
#include "objects/loxstring.h"
#include "typechecker/global_name_resolver.h"
#include "typechecker/expressions/and_expression.h"
#include "typechecker/expressions/assignment_expression.h"
#include "typechecker/expressions/as_expression.h"
#include "typechecker/expressions/binary_expression.h"
#include "typechecker/expressions/call_expression.h"
#include "typechecker/expressions/is_expression.h"
#include "typechecker/expressions/or_expression.h"
#include "typechecker/expressions/primary_expression.h"
#include "typechecker/statements/expression_statement.h"
#include "typechecker/statements/fun_statement.h"
#include "typechecker/statements/if_statement.h"
#include "typechecker/statements/print_statement.h"
#include "typechecker/statements/return_statement.h"
#include "typechecker/statements/var_statement.h"
#include "typechecker/types/function_type.h"
#include "typechecker/types/union_type.h"

std::shared_ptr<LoxClosure> AstCompiler::compile(std::string &&source) {
    StatementParser parser(std::move(source));

    try {
        statements = parser.parse_all();
    } catch (StructureParsingError &error) {
        std::cerr << error.what() << std::endl;
        return nullptr;
    }

    for (auto &each: statements) {
        visit_statement(each);
    }
    if (has_error) {
        return nullptr;
    }
    return main;
}

void AstCompiler::visit_block_statement(BlockStatement *stmt) {
    const auto local_count = scope->step_into();
    for (auto &each: stmt->body) {
        visit_statement(each);
    }
    const auto clear_amount = scope->step_out(local_count);
    if (clear_amount > 0) {
        current_chunk().write_opcode(Opcode::ClearN, -1);
        current_chunk().write_operand_1(clear_amount, -1);
    }
}

void AstCompiler::visit_expression_statement(ExpressionStatement *stmt) {
    stmt->expr->resolve_type(scope);
    visit_expression(stmt->expr);
    current_chunk().write_opcode(Opcode::Pop, stmt->expr->line);
}

void AstCompiler::visit_fun_statement(FunStatement *fun) {
    scope = std::make_shared<ST_Scope>(nullptr, fun->type);
    scope->step_into();

    scope->function_->set_name(fun->fun_name.get_lexeme());

    for (auto &parameter: fun->parameters) {
        // 将每一个参数作为本地变量处理
        scope->add_local(parameter.first, parameter.second);
        scope->initialize();
    }
    bool accepted_return = false;

    const auto &fun_return_type = fun->type->return_type;

    // 处理函数体内的每个语句
    for (auto &stmt: fun->body) {
        visit_statement(stmt);

        // 如果该函数有标注返回值，那么需要检查每个语句是否成功返回了合适的返回值
        // 这是为了保证函数返回正确的返回值。而return语句中的检查则是为了避免返回错误的返回值
        // 在第一个成功返回处，停止代码生成
        if (fun_return_type->type_enum != LoxTypeEnum::Void) {
            const auto &stmt_type = stmt->resolve_return_type();
            if (stmt_type->type_enum != LoxTypeEnum::Unspecified) {
                // 有返回值！不检察返回值类型是否匹配，因为return语句本身会检察
                accepted_return = true;
                break;
            }
        }
    }

    if (fun_return_type->type_enum != LoxTypeEnum::Void && accepted_return == false) {
        // 如果有标注返回值，但函数体内没有返回，则抛出异常
        throw MismatchedTypeError(fmt::format("for function {}, not all control flows return the expected type: '{}'",
                                              fun->fun_name.get_lexeme(), fun_return_type->to_string()));
    } else {
        // 如果没有标注返回值，那么返回nil（但这里的nil只是占位符，让 return 指令实现起来更简单），typechecker 会阻止真的试图使用它的人
        current_chunk().write_opcode(Opcode::LoadNil, -1);
        current_chunk().write_opcode(Opcode::Return, -1);
    }
    const uint8_t clear_amount = scope->step_out(0);
    if (clear_amount > 0) {
        // todo: 应该可以省略，因为函数结束会自动清除栈帧
        current_chunk().write_opcode(Opcode::ClearN, -1);
        current_chunk().write_operand(clear_amount, -1);
    }

    Runtime::record_allocation(scope->function_); // todo: 考虑 st_runtime

    auto [type, index] = name_resolver.resolve_name(fun->fun_name.get_lexeme());

    auto closure = std::make_shared<LoxClosure>(scope->function_);

    st_runtime.define_global(index, closure);
    Runtime::record_allocation(closure);

    if (fun->fun_name.get_lexeme() == "main") {
        if (main == nullptr) {
            main = closure;
        } else {
            IMPL_ERROR("multiple main definition should have been detected during the ast buildup");
        }
    }

    if (Flag::disassembly && has_error == false) {
        Disassembler disassembler(&scope->function_->get_chunk(), &std::cout);
        disassembler.disassemble(fun->fun_name.get_lexeme());
    }


    scope = std::move(scope->outer_);
}

void AstCompiler::visit_var_statement(VarStatement *stmt) {
    if (stmt->type->type_enum == LoxTypeEnum::Unspecified) {
        DEBUG_ASSERT(stmt->initializer != nullptr, "both initializer and type are empty");
        // 如果没有指明类型，则类型由初始值决定
        const auto type = stmt->initializer->resolve_type(scope);
        scope->add_local(stmt->name, type);
        visit_expression(stmt->initializer); // 将初始值入栈
    } else {
        // 如果指明了类型，那么初始值可能为空
        scope->add_local(stmt->name, stmt->type);

        if (stmt->initializer != nullptr) {
            // 初始值不为空, 判断指定的类型是否和初始值的实际类型匹配
            const auto type = stmt->initializer->resolve_type(scope);
            if (stmt->type->accept(type)) {
                visit_expression(stmt->initializer);
            } else {
                throw MismatchedTypeError(fmt::format(
                    "line {}: annotated type: '{}' does not match initializer type: '{}'",
                    stmt->name.get_line(),
                    stmt->type->to_string(), type->to_string()));
            }
        } else {
            // 初始值为空，此时，初始值由指定的类型决定。但也有些类型不允许初始值
            const int line = stmt->name.get_line();
            switch (stmt->type->type_enum) {
                case LoxTypeEnum::Int: {
                    const auto imme_index = Chunk::to_immediate(0);
                    current_chunk().write_opcode(Opcode::LoadImmediate, line);
                    current_chunk().write_operand(imme_index.value(), line);
                    break;
                }
                case LoxTypeEnum::Float: {
                    const auto imme_index = Chunk::to_immediate(0.0);
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
                    const auto u = std::static_pointer_cast<UnionType>(stmt->type);
                    if (u->contains_nil()) {
                        current_chunk().write_opcode(Opcode::LoadNil, line);
                    } else {
                        throw NoInitializationError(
                            fmt::format("line {}: the annotated type: '{}' must be initialized", stmt->name.get_line(),
                                        stmt->type->to_string()));
                    }
                    break;
                }
                default:
                    throw NoInitializationError(fmt::format("line {}: the annotated type: '{}' must be initialized",
                                                            stmt->name.get_line(),
                                                            stmt->type->to_string()));
            }
        }
    }
    scope->initialize();
}

void AstCompiler::visit_print_statement(PrintStatement *stmt) {
    stmt->expr->resolve_type(scope);
    visit_expression(stmt->expr);
    emit_opcode(Opcode::Print, stmt->expr->get_line());
}

void AstCompiler::visit_return_statement(ReturnStatement *return_statement) {
    if (return_statement->value == nullptr) {
        // 无返回值
        if (scope->function_->get_type()->return_type->type_enum == LoxTypeEnum::Void) {
            current_chunk().write_opcode(Opcode::LoadNil, return_statement->line);
            current_chunk().write_opcode(Opcode::Return, return_statement->line);
        } else {
            throw MismatchedTypeError(fmt::format("line {}: the function expects return type of '{}', but got '{}'",
                                                  return_statement->line,
                                                  scope->function_->get_type()->return_type->to_string(),
                                                  "void"));
        }
    }
    const auto type = return_statement->value->resolve_type(scope);
    return_statement->cached_return_type = type;
    if (scope == nullptr) {
        throw DefinitionPositionError(fmt::format("line {}: can only return inside a function",
                                                  return_statement->value->get_line()));
    }
    if (scope->function_->get_type()->return_type->accept(type) == false) {
        throw MismatchedTypeError(fmt::format("line {}: the function expects return type of '{}', but got '{}'",
                                              return_statement->value->get_line(),
                                              scope->function_->get_type()->return_type->to_string(),
                                              type->to_string()
        ));
    }
    visit_expression(return_statement->value);
    current_chunk().write_opcode(Opcode::Return, return_statement->value->get_line());
}

void AstCompiler::visit_if_statement(IfStatement *if_statement) {
    const auto type = if_statement->condition->resolve_type(scope);
    if (type->type_enum != LoxTypeEnum::Bool) {
        throw MismatchedTypeError(fmt::format("line {}: the condition must be bool, but got {}",
                                              if_statement->condition->line, type->to_string()));
    }
    visit_expression(if_statement->condition);
    const auto to_else = emit_jump(Opcode::JumpIfPopFalse, if_statement->condition->line);
    visit_statement(if_statement->then_branch);
    if (if_statement->else_branch != nullptr) {
        const auto to_end = emit_jump(Opcode::Jump, -1);
        patch_jump(to_else);
        visit_statement(if_statement->else_branch);
        patch_jump(to_end);
    } else {
        patch_jump(to_else);
    }
}

void AstCompiler::visit_and_expr(AndExpression *expr) {
    visit_expression(expr->left);
    const auto short_circuit = emit_jump(Opcode::JumpIfFalse, -1);
    visit_expression(expr->right);
    patch_jump(short_circuit);
}

void AstCompiler::visit_as_expr(AsExpression *expr) {
    visit_expression(expr->expr);
    const uint16_t index = current_chunk().add_constant(expr->as_type);
    current_chunk().write_opcode(Opcode::As, expr->line);
    current_chunk().write_operand_2(index, expr->line);
}

void AstCompiler::visit_is_expr(IsExpression *expr) {
    visit_expression(expr->value);
    const uint16_t index = current_chunk().add_constant(expr->test_type);
    current_chunk().write_opcode(Opcode::Is, expr->line);
    current_chunk().write_operand_2(index, expr->line);
}

void AstCompiler::visit_assignment_expr(AssignmentExpression *expr) {
    visit_expression(expr->right);
    const auto target_type = expr->left->get_expression_type();
    if (target_type == Expression::ExpressionType::Primary) {
        const auto left = std::unique_ptr<PrimaryExpression>(static_cast<PrimaryExpression *>(expr->left.release()));
        if (left->value_token.get_type() != TokenType::IDENTIFIER) {
            throw InvalidAssignmentTargetError(fmt::format("line {}: invalid assignment target", expr->left->line));
        }
        const Token &target_name = left->value_token;
        const auto local_found = scope->resolve_local(target_name);
        if (local_found.has_value() == false) {
            throw VariableNotFoundError(fmt::format("line {}: no such variable: '{}'",
                                                    target_name.get_line(),
                                                    target_name.get_lexeme()));
        }
        emit_opcode(Opcode::SetLocal, left->get_line());
        emit_operand_1(local_found.value(), left->get_line());
    } else {
        NOT_IMPLEMENTED();
    }
}

void AstCompiler::visit_binary_expr(BinaryExpression *expr) {
    visit_expression(expr->left);
    visit_expression(expr->right);
    const int line = expr->op.get_line();
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

void AstCompiler::visit_call_expr(CallExpression *expr) {
    visit_expression(expr->callee);
    for (auto &arg: expr->arguments) {
        visit_expression(arg);
    }
    current_chunk().write_opcode(Opcode::Call, expr->callee->get_line());
    current_chunk().write_operand_1(expr->arguments.size(), expr->callee->line);
    // NOT_IMPLEMENTED();
}

void AstCompiler::visit_or_expr(OrExpression *expr) {
    visit_expression(expr->left);
    const auto short_circuit = emit_jump(Opcode::JumpIfTrue, -1);
    visit_expression(expr->right);
    patch_jump(short_circuit);
}

void AstCompiler::visit_primary_expr(PrimaryExpression *expr) {
    const std::string lexeme = expr->value_token.get_lexeme();
    const int line = expr->value_token.get_line();
    expr->line = line;
    switch (expr->value_token.get_type()) {
        case TokenType::INTEGER: {
            int64_t num = std::stoll(lexeme);
            const auto index = Chunk::to_immediate(num);
            if (index.has_value()) {
                current_chunk().write_opcode(Opcode::LoadImmediate, line);
                current_chunk().write_operand_1(index.value(), line);
            } else {
                emit_constant(num, line);
            }
            break;
        }
        case TokenType::FLOAT: {
            double num = std::stod(lexeme);
            const auto index = Chunk::to_immediate(num);
            if (index.has_value()) {
                current_chunk().write_opcode(Opcode::LoadImmediate, line);
                current_chunk().write_operand_1(index.value(), line);
            } else {
                emit_constant(num, line);
            }
            break;
        }
        case TokenType::False:
            current_chunk().write_opcode(Opcode::LoadFalse, line);
            break;
        case TokenType::True:
            current_chunk().write_opcode(Opcode::LoadTrue, line);
            break;
        case TokenType::Nil:
            current_chunk().write_opcode(Opcode::LoadNil, line);
            break;
        case TokenType::STRING: {
            LoxReference value = Runtime::allocate_as_ref<LoxString>(lexeme.substr(1, lexeme.size() - 2));
            Runtime::record_allocation(value);
            emit_constant(value, line);
            break;
        }
        // todo: fmtstring

        case TokenType::IDENTIFIER: {
            const auto local_index = scope->resolve_local(expr->value_token);
            // 先在本地寻找
            if (local_index.has_value()) {
                current_chunk().write_opcode(Opcode::LoadLocal, line);
                current_chunk().write_operand_1(local_index.value(), line);
                break;
            }
            auto [type, global_index] = name_resolver.resolve_name(lexeme);
            if (type->type_enum != LoxTypeEnum::Unspecified) {
                current_chunk().write_opcode(Opcode::LoadGlobal, line);
                current_chunk().write_operand_2(global_index, line);
                break;
            }
            throw VariableNotFoundError(fmt::format("line {}: no such variable: {}", expr->value_token.get_line(),
                                                    lexeme));
            break;
        }
        default:
            ASSERT_UNREACHABLE();
    }
}

void AstCompiler::visit_unary_expr(UnaryExpression *expr) {
    visit_expression(expr->operand);
    if (expr->op.get_type() == TokenType::MINUS) {
        emit_opcode(Opcode::Negate, expr->get_line());
    } else if (expr->op.get_type() == TokenType::BANG) {
        emit_opcode(Opcode::Not, expr->get_line());
    } else {
        ASSERT_UNREACHABLE();
    }
}

Chunk &AstCompiler::current_chunk() {
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

void AstCompiler::emit_constant(const Value &value, int line) {
    const uint16_t index = current_chunk().add_constant(value);
    if (within<uint8_t>(index)) {
        current_chunk().write_opcode(Opcode::LoadConstant, line);
        current_chunk().write_operand_1(index, line);
    } else if (within<uint16_t>(index)) {
        current_chunk().write_opcode(Opcode::LoadConstant2, line);
        current_chunk().write_operand_2(index, line);
    } else {
        ASSERT_UNREACHABLE();
    }
}

OperandSize AstCompiler::emit_jump(Opcode jump_operation, int line) {
    current_chunk().write_opcode(jump_operation, line);
    current_chunk().write_operand_2(0, line);
    return current_chunk().code_size() - 2;
}

void AstCompiler::patch_jump(OperandSize from_label) {
    const size_t distance = current_chunk().code_size() - from_label - 2;
    if (!within<uint16_t>(distance)) {
        throw JumpDistanceOverflowError("the distance to jump is too much to be encoded as an uint16");
    }
    auto [high, low] = u16_to_u8(distance);
    current_chunk().code_at(from_label) = low;
    current_chunk().code_at(from_label + 1) = high;
}

void AstCompiler::loop_back(size_t destination) {
    DEBUG_ASSERT(current_chunk().code_size() >= destination, "loop back is jumping forward!");
    const size_t distance = current_chunk().code_size() - destination + 3;

    if (!within<uint16_t>(distance)) {
        throw JumpDistanceOverflowError("the distance to jump is too much to be encoded as an uint16");
    }
    emit_opcode(Opcode::JumpBack, 0);
    emit_operand_2(distance, 0);
}

bool AstCompiler::is_global_scope() const {
    return scope == nullptr;
}
