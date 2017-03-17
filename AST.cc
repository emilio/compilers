#include "AST.h"
#include <cmath>

namespace ast {

Value BinaryOperation::evaluate() const {
  Value left = m_lhs->evaluate();
  Value right = m_rhs->evaluate();

#define IMPL_BIN_OP(ch, op)                                                    \
  if (m_op == ch) {                                                            \
    if (left.type() != right.type())                                           \
      return Value::createDouble(left.normalizedValue() op                     \
                                 right.normalizedValue());                     \
    switch (left.type()) {                                                     \
      case ValueType::Integer:                                                 \
        return Value::createInt(left.intValue() op right.intValue());          \
      case ValueType::Float:                                                   \
        return Value::createDouble(left.doubleValue() op right.doubleValue()); \
    }                                                                          \
  }

  IMPL_BIN_OP('+', +) // Nasty
  IMPL_BIN_OP('-', -)

#undef IMPL_BIN_OP

  assert(false); // Again, easily reachable, need to implement other operators.
  return Value::createInt(0);
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

Value FunctionCall::evaluate() const {
  if (m_name == "cos" && m_arguments.size() == 1) {
    double val = m_arguments[0]->evaluate().normalizedValue();
    return Value::createDouble(cos(val));
  }

  assert(false); // TODO(emilio): This is actually pretty reachable.
  return Value::createDouble(0.0);
}

}  // namespace ast
