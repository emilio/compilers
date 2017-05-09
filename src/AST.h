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

#pragma once

#include <cassert>
#include <memory>
#include <vector>

#include "ASTDumper.h"
#include "Result.h"
#include "Tokenizer.h"
#include "Value.h"

class BytecodeCollector;

namespace ast {

enum class NodeType {
#define NODE_TYPE(ty) ty,
#include "ASTNodeTypes.h"
#undef NODE_TYPE
};

class ASTEvaluatorContext;

/**
 * Returns whether the given bytecode collection resulted in a new value being
 * at the top of the stack.
 */
enum class BytecodeCollectionStatus {
  PushedToStack,
  DidntPush,
};

using BytecodeCollectionResult = Result<BytecodeCollectionStatus, std::string>;

class Node {
 public:
  virtual bool isOfType(NodeType) const = 0;
  virtual const char* name() const = 0;
  virtual void dump(ASTDumper) const = 0;
  virtual ~Node() = default;
  virtual BytecodeCollectionResult toByteCode(BytecodeCollector&) const {
    return std::string("Bytecode generation not implemented yet for ") + name();
  }
};

class Expression : public Node {
 public:
  bool isOfType(NodeType type) const override {
    return type == NodeType::Expression;
  }
};

class VariableBinding final : public Expression {
  std::string m_name;

 public:
  explicit VariableBinding(const char* name) : m_name(name) {}

  const char* name() const final { return "VariableBinding"; }

  const std::string& varName() const { return m_name; }

  void dump(ASTDumper) const final;

  bool isOfType(NodeType type) const final {
    return type == NodeType::VariableBinding || Expression::isOfType(type);
  }

  BytecodeCollectionResult toByteCode(BytecodeCollector&) const override;
};

class ConstantExpression final : public Expression {
  Value m_value;

 public:
  explicit ConstantExpression(Value value) : m_value(value) {}

  const char* name() const final { return "ConstantExpression"; }
  void dump(ASTDumper) const final;

  bool isOfType(NodeType type) const override {
    return type == NodeType::ConstantExpression || Expression::isOfType(type);
  }

  BytecodeCollectionResult toByteCode(BytecodeCollector&) const override;
};

class UnaryOperation final : public Expression {
  Operator m_op;
  std::unique_ptr<Expression> m_rhs;

 public:
  UnaryOperation(Operator op, std::unique_ptr<Expression>&& expr)
      : m_op(op), m_rhs(std::move(expr)) {}

  const char* name() const final { return "UnaryOperation"; }
  void dump(ASTDumper) const final;

  bool isOfType(NodeType type) const override {
    return type == NodeType::UnaryOperation || Expression::isOfType(type);
  }

  BytecodeCollectionResult toByteCode(BytecodeCollector&) const override;
};

// A statement is an expression terminated by a semicolon.
//
// Evaluating this will always yield a zero, but it may have side effects like
// setting variable bindings.
class Statement final : public Expression {
  std::unique_ptr<Expression> m_inner;

 public:
  explicit Statement(std::unique_ptr<Expression>&& inner)
      : m_inner(std::move(inner)) {}

  const char* name() const final { return "Statement"; }

  bool isOfType(NodeType type) const final {
    return type == NodeType::Statement || Expression::isOfType(type);
  }

  void dump(ASTDumper) const final;

  BytecodeCollectionResult toByteCode(BytecodeCollector&) const override;
};

// A block is a list of statements, with a final expression, potentially.
class Block final : public Expression {
  std::vector<std::unique_ptr<Statement>> m_statements;
  std::unique_ptr<Expression> m_lastExpression;  // may be null.
 public:
  explicit Block(std::vector<std::unique_ptr<Statement>>&& statements,
                 std::unique_ptr<Expression>&& lastExpression)
      : m_statements(std::move(statements)),
        m_lastExpression(std::move(lastExpression)) {}

  const char* name() const final { return "Block"; }

  bool isOfType(NodeType type) const final {
    return type == NodeType::Block || Expression::isOfType(type);
  }

  void dump(ASTDumper) const final;

  BytecodeCollectionResult toByteCode(BytecodeCollector&) const override;
};

class BinaryOperation final : public Expression {
  Operator m_op;
  std::unique_ptr<Expression> m_lhs;
  std::unique_ptr<Expression> m_rhs;

 public:
  BinaryOperation(Operator op,
                  std::unique_ptr<Expression>&& lhs,
                  std::unique_ptr<Expression>&& rhs)
      : m_op(op), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) {}

  const char* name() const final { return "BinaryOperation"; }
  void dump(ASTDumper) const final;

  bool isOfType(NodeType type) const override {
    return type == NodeType::BinaryOperation || Expression::isOfType(type);
  }

  BytecodeCollectionResult toByteCode(BytecodeCollector&) const override;
};

class FunctionCall final : public Expression {
  std::string m_name;
  std::vector<std::unique_ptr<Expression>> m_arguments;

 public:
  FunctionCall(std::string&& name,
               std::vector<std::unique_ptr<Expression>>&& args)
      : m_name(std::move(name)), m_arguments(std::move(args)) {}

  const char* name() const final { return "FunctionCall"; }
  void dump(ASTDumper) const final;

  bool isOfType(NodeType type) const override {
    return type == NodeType::FunctionCall || Expression::isOfType(type);
  }

  BytecodeCollectionResult toByteCode(BytecodeCollector&) const final;
};

class ParenthesizedExpression final : public Expression {
  std::unique_ptr<Expression> m_inner;

 public:
  ParenthesizedExpression(std::unique_ptr<Expression>&& inner)
      : m_inner(std::move(inner)) {}

  const char* name() const final { return "ParenthesizedExpression"; }
  void dump(ASTDumper) const final;

  bool isOfType(NodeType type) const override {
    return type == NodeType::ParenthesizedExpression ||
           Expression::isOfType(type);
  }

  BytecodeCollectionResult toByteCode(BytecodeCollector&) const override;
};

class ConditionalExpression final : public Expression {
  // May be null if it's an `else` clause.
  std::unique_ptr<Expression> m_condition;

  std::unique_ptr<Expression> m_innerExpression;

  // May be null, if there's no `else` or `else if` branch.
  std::unique_ptr<ConditionalExpression> m_else;

 public:
  ConditionalExpression(std::unique_ptr<Expression>&& condition,
                        std::unique_ptr<Expression>&& inner,
                        std::unique_ptr<ConditionalExpression>&& elseBranch)
      : m_condition(std::move(condition)),
        m_innerExpression(std::move(inner)),
        m_else(std::move(elseBranch)) {}

  const char* name() const final { return "ConditionalExpression"; }
  void dump(ASTDumper) const final;

  bool isOfType(NodeType type) const override {
    return type == NodeType::ConditionalExpression ||
           Expression::isOfType(type);
  }
};

class ForLoop final : public Expression {
  std::unique_ptr<ast::Expression> m_init;
  std::unique_ptr<ast::Expression> m_condition;
  std::unique_ptr<ast::Expression> m_afterClause;
  std::unique_ptr<ast::Expression> m_body;

 public:
  ForLoop(std::unique_ptr<ast::Expression>&& init,
          std::unique_ptr<ast::Expression>&& condition,
          std::unique_ptr<ast::Expression>&& afterClause,
          std::unique_ptr<ast::Expression>&& body)
      : m_init(std::move(init)),
        m_condition(std::move(condition)),
        m_afterClause(std::move(afterClause)),
        m_body(std::move(body)) {}

  const char* name() const final { return "ForLoop"; }

  void dump(ASTDumper) const final;

  bool isOfType(NodeType type) const override {
    return type == NodeType::ForLoop || Expression::isOfType(type);
  }
};

#define NODE_TYPE(ty)                                                          \
  inline bool is##ty(const Node& node) { return node.isOfType(NodeType::ty); } \
  inline bool is##ty(const Node* node) { return node && is##ty(*node); }       \
                                                                               \
  inline ty& to##ty(Node& node) {                                              \
    assert(is##ty(node));                                                      \
    return static_cast<ty&>(node);                                             \
  }                                                                            \
                                                                               \
  inline const ty& to##ty(const Node& node) {                                  \
    assert(is##ty(node));                                                      \
    return static_cast<const ty&>(node);                                       \
  }                                                                            \
                                                                               \
  inline ty* to##ty(Node* node) {                                              \
    assert(!node || is##ty(*node));                                            \
    return static_cast<ty*>(node);                                             \
  }                                                                            \
                                                                               \
  inline const ty* to##ty(const Node* node) {                                  \
    assert(!node || is##ty(*node));                                            \
    return static_cast<const ty*>(node);                                       \
  }

#include "ASTNodeTypes.h"
#undef NODE_TYPE

}  // namespace ast
