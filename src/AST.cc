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

namespace ast {

void ConstantExpression::dump(ASTDumper dumper) const {
  dumper << name() << " " << m_value;
}

Value BinaryOperation::evaluate() const {
  Value left = m_lhs->evaluate();
  Value right = m_rhs->evaluate();

#define IMPL_BIN_OP(ch, op)                                                    \
  if (m_op == ch) {                                                            \
    if (left.type() != right.type())                                           \
      return Value::createDouble(left.normalizedValue()                        \
                                     op right.normalizedValue());              \
    switch (left.type()) {                                                     \
      case ValueType::Integer:                                                 \
        return Value::createInt(left.intValue() op right.intValue());          \
      case ValueType::Float:                                                   \
        return Value::createDouble(left.doubleValue() op right.doubleValue()); \
    }                                                                          \
  }

  IMPL_BIN_OP('+', +)  // Nasty
  IMPL_BIN_OP('-', -)
  IMPL_BIN_OP('*', *)
  IMPL_BIN_OP('/', /)

#undef IMPL_BIN_OP

  assert(false);  // Again, easily reachable, need to implement other operators.
  return Value::createInt(0);
}

void BinaryOperation::dump(ASTDumper dumper) const {
  dumper << name() << "(" << m_op << ")";
  m_lhs->dump(dumper);
  m_rhs->dump(dumper);
}

Value UnaryOperation::evaluate() const {
  Value inner = m_rhs->evaluate();

  if (m_op == '+')
    return inner;

  if (m_op == '-') {
    switch (inner.type()) {
      case ValueType::Integer:
        return Value::createInt(-inner.intValue());
      case ValueType::Float:
        return Value::createDouble(-inner.doubleValue());
    }
  }

  assert(false);
  return Value::createInt(0);
}

void UnaryOperation::dump(ASTDumper dumper) const {
  dumper << name() << "(" << m_op << ")";
  m_rhs->dump(dumper);
}

// I didn't chose the name, the teacher did :)
inline double sqr(double of) {
  return ::sqrt(of);
}

Value FunctionCall::evaluate() const {
#define IMPL_ONE_ARG_FN(fn_name)                               \
  if (m_name == #fn_name && m_arguments.size() == 1) {         \
    double val = m_arguments[0]->evaluate().normalizedValue(); \
    return Value::createDouble(fn_name(val));                  \
  }

  IMPL_ONE_ARG_FN(sin);
  IMPL_ONE_ARG_FN(cos);
  IMPL_ONE_ARG_FN(abs);
  IMPL_ONE_ARG_FN(sqr);

  // Just to test sqr easily.
  if (m_name == "pow" && m_arguments.size() == 2) {
    double val = m_arguments[0]->evaluate().normalizedValue();
    double exp = m_arguments[1]->evaluate().normalizedValue();
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
