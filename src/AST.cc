/*
 * Copyright (C) 2017 Emilio Cobos Álvarez <emilio@crisal.io>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "AST.h"
#include "BytecodeCollector.h"
#include <cmath>

namespace ast {

void ConstantExpression::dump(ASTDumper dumper) const {
  dumper << name() << " " << m_value;
}

void VariableBinding::dump(ASTDumper dumper) const {
  dumper << name() << " " << m_name;
}

void BinaryOperation::dump(ASTDumper dumper) const {
  dumper << name() << "(" << m_op << ")";
  m_lhs->dump(dumper);
  m_rhs->dump(dumper);
}

void UnaryOperation::dump(ASTDumper dumper) const {
  dumper << name() << "(" << m_op << ")";
  m_rhs->dump(dumper);
}

void Statement::dump(ASTDumper dumper) const {
  dumper << name();
  m_inner->dump(dumper);
}

void Block::dump(ASTDumper dumper) const {
  dumper << name();
  for (const auto& statement : m_statements)
    statement->dump(dumper);
  if (m_lastExpression)
    m_lastExpression->dump(dumper);
}

void FunctionCall::dump(ASTDumper dumper) const {
  dumper << name() << "(" << m_name << ")";
  for (const auto& arg : m_arguments)
    arg->dump(dumper);
}

void ParenthesizedExpression::dump(ASTDumper dumper) const {
  dumper << name();
  m_inner->dump(dumper);
}

void ConditionalExpression::dump(ASTDumper dumper) const {
  dumper << name();
  if (m_condition)
    m_condition->dump(dumper);
  m_innerExpression->dump(dumper);
  if (m_else)
    m_else->dump(dumper);
}

void ForLoop::dump(ASTDumper dumper) const {
  dumper << name();
  if (m_init)
    m_init->dump(dumper);
  if (m_condition)
    m_condition->dump(dumper);
  if (m_afterClause)
    m_afterClause->dump(dumper);
  m_body->dump(dumper);
}

BytecodeCollectionResult
ConstantExpression::toByteCode(BytecodeCollector& collector) const {
  collector.pushToStack(m_value);
  return BytecodeCollectionStatus::PushedToStack;
}

BytecodeCollectionResult
UnaryOperation::toByteCode(BytecodeCollector& collector) const {
  assert(m_op == Operator::Plus || m_op == Operator::Minus);

  BytecodeCollectionStatus status;
  if (m_op == Operator::Minus) {
    collector.pushToStack(Value::createDouble(0.));
    TRY_VAR(status, m_rhs->toByteCode(collector));
    if (status != BytecodeCollectionStatus::PushedToStack)
      return std::string("Expected an expression with a value");
    collector.binOp(m_op);
    return BytecodeCollectionStatus::PushedToStack;
  }

  TRY_VAR(status, m_rhs->toByteCode(collector));
  if (status != BytecodeCollectionStatus::PushedToStack)
    return std::string("Expected an expression with a value");
  return BytecodeCollectionStatus::PushedToStack;
}

BytecodeCollectionResult
Statement::toByteCode(BytecodeCollector& collector) const {
  BytecodeCollectionStatus status;
  TRY_VAR(status, m_inner->toByteCode(collector));
  if (status == BytecodeCollectionStatus::PushedToStack)
    collector.popFromStack();
  return BytecodeCollectionStatus::DidntPush;
}

BytecodeCollectionResult
Block::toByteCode(BytecodeCollector& collector) const {
  BytecodeCollectionStatus status;
  for (const auto& statement : m_statements) {
    TRY_VAR(status, statement->toByteCode(collector));
    assert(status == BytecodeCollectionStatus::DidntPush);
  }

  if (m_lastExpression)
    return m_lastExpression->toByteCode(collector);
  return BytecodeCollectionStatus::DidntPush;
}

BytecodeCollectionResult
BinaryOperation::toByteCode(BytecodeCollector& collector) const {
  BytecodeCollectionStatus status;
  TRY_VAR(status, m_lhs->toByteCode(collector));
  if (status != BytecodeCollectionStatus::PushedToStack)
    return std::string("Expected lhs of expression to leave a value in the stack");
  TRY_VAR(status, m_rhs->toByteCode(collector));
  if (status != BytecodeCollectionStatus::PushedToStack)
    return std::string("Expected lhs of expression to leave a value in the stack");
  collector.binOp(m_op);
  return BytecodeCollectionStatus::PushedToStack;
}

BytecodeCollectionResult
ParenthesizedExpression::toByteCode(BytecodeCollector& collector) const {
  return m_inner->toByteCode(collector);
}

}  // namespace ast
