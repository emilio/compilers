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
#include <cmath>
#include "BytecodeCollector.h"

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

static Optional<BuiltinFunction>
builtinFunctionFromName(const std::string& name) {
  if (name == "cos")
    return Some(BuiltinFunction::Cos);
  if (name == "sin")
    return Some(BuiltinFunction::Sin);
  if (name == "abs")
    return Some(BuiltinFunction::Abs);
  if (name == "sqrt")
    return Some(BuiltinFunction::Sqrt);
  if (name == "pow")
    return Some(BuiltinFunction::Pow);
  return None;
}

BytecodeCollectionResult
FunctionCall::toByteCode(BytecodeCollector& collector) const {
  auto functionId = builtinFunctionFromName(m_name);
  if (!functionId)
    return std::string("Unknown function: ") + m_name;
  BytecodeCollectionStatus status = BytecodeCollectionStatus::DidntPush;
  for (auto it = m_arguments.rbegin(); it != m_arguments.rend(); ++it) {
    TRY_VAR(status, (*it)->toByteCode(collector));
    if (status == BytecodeCollectionStatus::DidntPush)
      return std::string("Argument didn't leave a value on the stack...");
  }
  collector.pushFunctionCall(*functionId, m_arguments.size());
  return BytecodeCollectionStatus::PushedToStack;
}


BytecodeCollectionResult ConstantExpression::toByteCode(
    BytecodeCollector& collector) const {
  collector.pushToStack(m_value);
  return BytecodeCollectionStatus::PushedToStack;
}

BytecodeCollectionResult UnaryOperation::toByteCode(
    BytecodeCollector& collector) const {
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

BytecodeCollectionResult Statement::toByteCode(
    BytecodeCollector& collector) const {
  BytecodeCollectionStatus status;
  TRY_VAR(status, m_inner->toByteCode(collector));
  if (status == BytecodeCollectionStatus::PushedToStack)
    collector.popFromStack();
  return BytecodeCollectionStatus::DidntPush;
}

BytecodeCollectionResult Block::toByteCode(BytecodeCollector& collector) const {
  BytecodeCollectionStatus status = BytecodeCollectionStatus::DidntPush;
  collector.pushScope();

  for (const auto& statement : m_statements) {
    TRY_VAR(status, statement->toByteCode(collector));
    assert(status == BytecodeCollectionStatus::DidntPush);
  }

  if (m_lastExpression)
    TRY_VAR(status, m_lastExpression->toByteCode(collector));

  collector.popScope();
  return status;
}

BytecodeCollectionResult BinaryOperation::toByteCode(
    BytecodeCollector& collector) const {
  BytecodeCollectionStatus status;

  if (m_op == Operator::Equals) {
    if (!isVariableBinding(*m_lhs))
      return std::string("Assigned to something that was not a variable");
    const VariableBinding& var = toVariableBinding(*m_lhs);
    LabelId id = collector.reserveVariableIdFor(var.varName());
    TRY_VAR(status, m_rhs->toByteCode(collector));
    if (status != BytecodeCollectionStatus::PushedToStack)
      return std::string(
          "Expected rhs of expression to leave a value "
          "in the stack");
    collector.pushAssignTo(id);
    return BytecodeCollectionStatus::PushedToStack;
  }

  TRY_VAR(status, m_lhs->toByteCode(collector));
  if (status != BytecodeCollectionStatus::PushedToStack)
    return std::string(
        "Expected lhs of expression to leave a value in the stack");
  TRY_VAR(status, m_rhs->toByteCode(collector));
  if (status != BytecodeCollectionStatus::PushedToStack)
    return std::string(
        "Expected lhs of expression to leave a value in the stack");
  collector.binOp(m_op);
  return BytecodeCollectionStatus::PushedToStack;
}

BytecodeCollectionResult VariableBinding::toByteCode(
    BytecodeCollector& collector) const {
  Optional<LabelId> id = collector.resolveVariable(varName());
  if (!id)
    return std::string("Unresolved variable: ") + varName();
  collector.pushLoadVar(*id);
  return BytecodeCollectionStatus::PushedToStack;
}

BytecodeCollectionResult ParenthesizedExpression::toByteCode(
    BytecodeCollector& collector) const {
  return m_inner->toByteCode(collector);
}

}  // namespace ast
