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
#include "ASTEvaluator.h"

namespace ast {

void ConstantExpression::dump(ASTDumper dumper) const {
  dumper << name() << " " << m_value;
}

void VariableBinding::dump(ASTDumper dumper) const {
  dumper << name() << " " << m_name;
}

Value VariableBinding::evaluate(ASTEvaluatorContext& ctx) const {
  Optional<Value> val = ctx.resolveVariableBinding(m_name);
  // TODO(emilio): Need to do proper error handling!
  if (!val)
    return Value::createInt(0);
  return *val;
}

Value BinaryOperation::evaluate(ASTEvaluatorContext& ctx) const {
  Value left = m_lhs->evaluate(ctx);
  Value right = m_rhs->evaluate(ctx);

#define IMPL_BIN_OP(operator_, op)                                             \
  if (m_op == Operator::operator_) {                                           \
    if (left.type() != right.type())                                           \
      return Value::createDouble(left.normalizedValue()                        \
                                     op right.normalizedValue());              \
    switch (left.type()) {                                                     \
      case ValueType::Integer:                                                 \
        return Value::createInt(left.intValue() op right.intValue());          \
      case ValueType::Float:                                                   \
        return Value::createDouble(left.doubleValue() op right.doubleValue()); \
      case ValueType::Bool:                                                    \
        return Value::createDouble(left.boolValue() op right.boolValue());     \
    }                                                                          \
  }

  IMPL_BIN_OP(Plus, +)  // Nasty
  IMPL_BIN_OP(Minus, -)
  IMPL_BIN_OP(Star, *)
  IMPL_BIN_OP(Slash, /)

#undef IMPL_BIN_OP

  assert(false);  // Again, easily reachable, need to implement other operators.
  return Value::createInt(0);
}

void BinaryOperation::dump(ASTDumper dumper) const {
  dumper << name() << "(" << m_op << ")";
  m_lhs->dump(dumper);
  m_rhs->dump(dumper);
}

Value UnaryOperation::evaluate(ASTEvaluatorContext& ctx) const {
  Value inner = m_rhs->evaluate(ctx);

  switch (m_op) {
    case Operator::Plus:
      return inner;
    case Operator::Minus:
      switch (inner.type()) {
        case ValueType::Integer:
          return Value::createInt(-inner.intValue());
        case ValueType::Float:
          return Value::createDouble(-inner.doubleValue());
        case ValueType::Bool:
          return inner;
      }
    default:
      assert(false && "Invalid unary operator!");
      return Value::createInt(0);
  }
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

Value FunctionCall::evaluate(ASTEvaluatorContext& ctx) const {
#define IMPL_ONE_ARG_FN(fn_name, matching_fn)                     \
  if (m_name == #fn_name && m_arguments.size() == 1) {            \
    double val = m_arguments[0]->evaluate(ctx).normalizedValue(); \
    return Value::createDouble(matching_fn(val));                 \
  }

  IMPL_ONE_ARG_FN(sin, sin);
  IMPL_ONE_ARG_FN(cos, cos);
  IMPL_ONE_ARG_FN(abs, fabs);
  IMPL_ONE_ARG_FN(sqr, sqrt);

  // Just to test sqr easily.
  if (m_name == "pow" && m_arguments.size() == 2) {
    double val = m_arguments[0]->evaluate(ctx).normalizedValue();
    double exp = m_arguments[1]->evaluate(ctx).normalizedValue();
    return Value::createDouble(pow(val, exp));
  }

  assert(false);  // TODO(emilio): This is actually pretty reachable.
  return Value::createDouble(0.0);
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

}  // namespace ast
