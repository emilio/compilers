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

#include "Parser.h"

#include <memory>

ast::Node* Parser::parse() {
  m_astRoot = parseExpression();

  auto tok = m_tokenizer.nextToken();
  if (!tok || tok->type() != TokenType::Eof)
    m_astRoot.reset();

  if (!m_astRoot)
    assert(m_parseError);
  return m_astRoot.get();
}

std::unique_ptr<ast::Expression> Parser::noteParseError(std::string&& message) {
  assert(!m_parseError);
  assert(!m_astRoot);
  m_parseError.reset(
      new ParseError(m_tokenizer.location(), std::move(message)));
  return nullptr;
}

Optional<Token> Parser::nextToken() {
  if (m_lastToken)
    return std::move(m_lastToken);
  return m_tokenizer.nextToken();
}

std::unique_ptr<ast::Expression> Parser::parseOneExpression() {
  Optional<Token> tok = nextToken();

  switch (tok->type()) {
    case TokenType::SemiColon:
      return noteParseError("Stray semicolon");
    case TokenType::Keyword: {
      return noteParseError("TODO");
    }
    case TokenType::Float:
    case TokenType::Number: {
      // Maybe it's a standalone token, maybe it's the lhs of an arbitrarily
      // long binary expression tree.
      Value val = tok->type() == TokenType::Number
                      ? Value::createInt(tok->number())
                      : Value::createDouble(tok->doubleValue());

      return std::make_unique<ast::ConstantExpression>(val);
    }
    case TokenType::LeftParen: {
      std::unique_ptr<ast::Expression> inner = parseExpression();
      if (!inner)
        return nullptr;
      Optional<Token> endingParen = nextToken();
      if (!endingParen || endingParen->type() != TokenType::RightParen)
        return noteParseError("Unbalanced paren");
      return std::make_unique<ast::ParenthesizedExpression>(std::move(inner));
    }
    case TokenType::Identifier: {
      std::vector<std::unique_ptr<ast::Expression>> arguments;
      std::string name = tok->ident();

      Optional<Token> tok = nextToken();
      if (!tok || tok->type() != TokenType::LeftParen) {
        m_lastToken = std::move(tok);
        return std::make_unique<ast::VariableBinding>(tok->ident());
      }

      // Otherwise this is a function call.
      tok = nextToken();
      if (!tok)
        return noteParseError(m_tokenizer.errorMessage());
      if (tok->type() != TokenType::RightParen) {
        m_lastToken = std::move(tok);

        while (true) {
          auto arg = parseExpression();
          if (!arg)
            return nullptr;
          arguments.push_back(std::move(arg));
          tok = nextToken();
          if (!tok)
            return noteParseError(m_tokenizer.errorMessage());
          if (tok->type() == TokenType::RightParen)
            break;
          if (tok->type() != TokenType::Comma)
            return noteParseError("Expected comma after argument");
        }
      }

      return std::make_unique<ast::FunctionCall>(std::move(name),
                                                 std::move(arguments));
    }
    case TokenType::RightParen:
      return noteParseError("Unbalanced paren");
    case TokenType::Comma:
      return noteParseError("Unexpected standalone comma");
    case TokenType::Operator: {
      Operator op = tok->op();
      auto target = parseExpression();
      if (!target)
        return nullptr;
      return std::make_unique<ast::UnaryOperation>(op, std::move(target));
    }
    case TokenType::Eof:
      return noteParseError("Unexpected EOF");
  }
  assert(false);
  noteParseError("Internal error");
  return nullptr;
}

static inline bool isProductOperation(Operator op) {
  return op == Operator::Star || op == Operator::Slash;
}

std::unique_ptr<ast::Expression> Parser::parseExpression() {
  auto expr = parseProduct();
  if (!expr)
    return nullptr;

  while (true) {
    auto tok = nextToken();
    if (!tok)
      return noteParseError(m_tokenizer.errorMessage());
    if (tok->type() != TokenType::Operator) {
      m_lastToken = std::move(tok);
      return expr;
    }
    assert(!isProductOperation(tok->op()));
    auto rhs = parseProduct();
    if (!rhs)
      return nullptr;
    expr = std::make_unique<ast::BinaryOperation>(tok->op(), std::move(expr),
                                                  std::move(rhs));
  }
}

std::unique_ptr<ast::Expression> Parser::parseProduct() {
  auto expr = parseOneExpression();
  if (!expr)
    return nullptr;

  while (true) {
    auto tok = nextToken();
    if (!tok)
      return noteParseError(m_tokenizer.errorMessage());
    if (tok->type() != TokenType::Operator || !isProductOperation(tok->op())) {
      m_lastToken = std::move(tok);
      return expr;
    }

    auto rhs = parseOneExpression();
    if (!rhs)
      return nullptr;
    expr = std::make_unique<ast::BinaryOperation>(tok->op(), std::move(expr),
                                                  std::move(rhs));
  }
}
