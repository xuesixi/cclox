//
// Created by Yue Xue  on 9/5/25.
//
#include "typechecker/ast.h"
#include "typechecker/expressions/unary_expression.h"
#include "chunk.h"
#include "scope.h"
#include "typechecker/expressions/and_expression.h"
#include "typechecker/expressions/assignment_expression.h"
#include "typechecker/expressions/binary_expression.h"
#include "typechecker/expressions/or_expression.h"

std::shared_ptr<LoxFunction> AstCompiler::compile(std::string &&source) {
    StatementParser parser(std::move(source));
    statements = parser.parse_all();
    return nullptr;
}

void AstCompiler::visit_and_expr(AndExpression *expr) {
    expr->resolve_type();
    visit_expression(expr->left);
    auto short_circuit = emit_jump(Opcode::JumpIfFalse, 0);
    visit_expression(expr->right);
    patch_jump(short_circuit);
}

void AstCompiler::visit_assignment_expr(AssignmentExpression *expr) {
    expr->resolve_type();

}

void AstCompiler::visit_binary_expr(BinaryExpression *expr) {
    expr->resolve_type();
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
    expr->resolve_type();
    visit_expression(expr->left);
    auto short_circuit = emit_jump(Opcode::JumpIfTrue, 0);
    visit_expression(expr->right);
    patch_jump(short_circuit);
}

void AstCompiler::visit_unary_expr(UnaryExpression *expr) {
    auto type = expr->resolve_type();
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
