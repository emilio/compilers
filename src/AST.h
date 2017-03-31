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
#include "Value.h"

namespace ast {

enum class NodeType {
#define NODE_TYPE(ty) ty,
#include "ASTNodeTypes.h"
#undef NODE_TYPE
};

class Node {
 public:
  virtual bool isOfType(NodeType) const = 0;
  virtual const char* name() const = 0;
  virtual void dump(ASTDumper) const = 0;
  virtual ~Node() = default;
};

class Expression : public Node {
 public:
  bool isOfType(NodeType type) const override {
    return type == NodeType::Expression;
  }

  virtual Value evaluate() const = 0;
};

class ConstantExpression final : public Expression {
  Value m_value;

 public:
  ConstantExpression(Value value) : m_value(value) {}

  const char* name() const final { return "ConstantExpression"; }
  void dump(ASTDumper) const final;

  bool isOfType(NodeType type) const override {
    return type == NodeType::ConstantExpression || Expression::isOfType(type);
  }

  Value evaluate() const final { return m_value; }
};

class UnaryOperation final : public Expression {
  char m_op;
  std::unique_ptr<Expression> m_rhs;

 public:
  UnaryOperation(char op, std::unique_ptr<Expression>&& expr)
      : m_op(op), m_rhs(std::move(expr)) {}

  const char* name() const final { return "UnaryOperation"; }
  void dump(ASTDumper) const final;

  bool isOfType(NodeType type) const override {
    return type == NodeType::UnaryOperation || Expression::isOfType(type);
  }

  Value evaluate() const final;
};

class BinaryOperation final : public Expression {
  char m_op;
  std::unique_ptr<Expression> m_lhs;
  std::unique_ptr<Expression> m_rhs;

 public:
  BinaryOperation(char op,
                  std::unique_ptr<Expression>&& lhs,
                  std::unique_ptr<Expression>&& rhs)
      : m_op(op), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) {}

  const char* name() const final { return "BinaryOperation"; }
  void dump(ASTDumper) const final;

  bool isOfType(NodeType type) const override {
    return type == NodeType::BinaryOperation || Expression::isOfType(type);
  }

  Value evaluate() const final;
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

  Value evaluate() const final;
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

  Value evaluate() const final { return m_inner->evaluate(); }
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
