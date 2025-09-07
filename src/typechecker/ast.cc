//
// Created by Yue Xue  on 9/5/25.
//
#include "typechecker/ast.h"
#include "typechecker/expressions/unary_expression.h"
#include "chunk.h"
#include "scope.h"

std::shared_ptr<LoxFunction> AstCompiler::compile(std::string &&source) {
    StatementParser parser(std::move(source));
    statements = parser.parse_all();
    return nullptr;
}

void AstCompiler::visit_unary_expr(UnaryExpression *expression) {
    auto type = expression->resolve_type();
    visit_expression(expression->operand.get());
    if (expression->op.get_type() == TokenType::MINUS) {
        emit_opcode(Opcode::Negate, expression->get_line());
    } else if (expression->op.get_type() == TokenType::BANG) {
        emit_opcode(Opcode::Not, expression->get_line());
    } else {
       NOT_IMPLEMENTED()
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
