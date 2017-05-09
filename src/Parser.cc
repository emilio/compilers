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
  // TODO(emilio): Create an anonymous block, and read a list of statements
  // instead?
  m_astRoot = parseExpression();

  auto tok = m_tokenizer.nextToken();
  if (!tok || tok->type() != TokenType::Eof) {
    m_astRoot.reset();
    noteParseError("Found unexpected token after program");
  }

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
      // TODO(emilio): C++ allows this, should we?
      return noteParseError("Stray semicolon");
    case TokenType::Keyword: {
      switch (tok->keyword()) {
        case Keyword::If: {
          Optional<Token> tok = nextToken();
          if (!tok || tok->type() != TokenType::LeftParen)
            return noteParseError(
                "Expected left parenthesis after if condition");
          std::unique_ptr<ast::Expression> condition = parseExpression();
          if (!condition)
            return nullptr;
          tok = nextToken();
          if (!tok || tok->type() != TokenType::RightParen)
            return noteParseError("Expected right paren after if condition");
          std::unique_ptr<ast::Expression> inner = parseExpression();
          if (!inner)
            return nullptr;
          std::unique_ptr<ast::ConditionalExpression> elseBranch =
              tryParseRemainingConditionalBranches();
          if (!elseBranch && m_parseError)
            return nullptr;
          return std::make_unique<ast::ConditionalExpression>(
              std::move(condition), std::move(inner), std::move(elseBranch));
        }
        case Keyword::Else:
          return noteParseError("extraneous else keyword");
        case Keyword::For: {
          Optional<Token> tok = nextToken();
          if (!tok || tok->type() != TokenType::LeftParen)
            return noteParseError("Expected left parenthesis after for clause");

          std::unique_ptr<ast::Expression> init;

          tok = nextToken();
          // Allow empty init clauses.
          if (!tok || tok->type() != TokenType::SemiColon) {
            m_lastToken = std::move(tok);
            init = parseExpression();
            if (!init)
              return nullptr;

            tok = nextToken();
            if (!tok || tok->type() != TokenType::SemiColon)
              return noteParseError("Expected semicolon after for init clause");
          }

          std::unique_ptr<ast::Expression> condition;
          tok = nextToken();
          if (!tok || tok->type() != TokenType::SemiColon) {
            m_lastToken = std::move(tok);
            condition = parseExpression();
            if (!condition)
              return nullptr;

            tok = nextToken();
            if (!tok || tok->type() != TokenType::SemiColon)
              return noteParseError("Expected semicolon after for condition");
          }

          tok = nextToken();
          std::unique_ptr<ast::Expression> afterClause;
          if (!tok || tok->type() != TokenType::RightParen) {
            m_lastToken = std::move(tok);
            afterClause = parseExpression();
            if (!afterClause)
              return nullptr;

            tok = nextToken();
            if (!tok || tok->type() != TokenType::RightParen)
              return noteParseError(
                  "Expected closing paren after for final clause");
          }

          std::unique_ptr<ast::Expression> body = parseExpression();
          if (!body)
            return nullptr;
          return std::make_unique<ast::ForLoop>(
              std::move(init), std::move(condition), std::move(afterClause),
              std::move(body));
        }
        case Keyword::While: {
          Optional<Token> tok = nextToken();
          if (!tok || tok->type() != TokenType::LeftParen)
            return noteParseError("Expected left parenthesis after for clause");
          std::unique_ptr<ast::Expression> condition = parseExpression();
          if (!condition)
            return nullptr;

          tok = nextToken();
          if (!tok || tok->type() != TokenType::RightParen)
            return noteParseError("Expected left paren after while condition");

          std::unique_ptr<ast::Expression> body = parseExpression();
          if (!body)
            return nullptr;

          return std::make_unique<ast::ForLoop>(nullptr, std::move(condition),
                                                nullptr, std::move(body));
        }
      }
      assert(false);
      return noteParseError("Internal error: Unexpected keyword");
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
    case TokenType::LeftBrace: {
      std::unique_ptr<ast::Expression> lastExpression;
      std::vector<std::unique_ptr<ast::Statement>> statements;
      while (true) {
        Optional<Token> closingBrace = nextToken();
        if (!closingBrace)
          return noteParseError("Unfinished block");
        if (closingBrace->type() == TokenType::RightBrace)
          return std::make_unique<ast::Block>(std::move(statements), nullptr);
        m_lastToken = std::move(closingBrace);
        std::unique_ptr<ast::Expression> inner = parseExpression();
        if (!inner)
          return nullptr;
        Optional<Token> semiColonOrBrace = nextToken();
        if (!semiColonOrBrace ||
            (semiColonOrBrace->type() != TokenType::SemiColon &&
             semiColonOrBrace->type() != TokenType::RightBrace)) {
          return noteParseError("Unbalanced block, or expected semicolon");
        }
        if (semiColonOrBrace->type() == TokenType::RightBrace)
          return std::make_unique<ast::Block>(std::move(statements),
                                              std::move(inner));
        statements.push_back(
            std::make_unique<ast::Statement>(std::move(inner)));
      }
    }
    case TokenType::Identifier: {
      std::vector<std::unique_ptr<ast::Expression>> arguments;
      std::string name = tok->ident();

      Optional<Token> tok = nextToken();
      if (!tok || tok->type() != TokenType::LeftParen) {
        m_lastToken = std::move(tok);
        return std::make_unique<ast::VariableBinding>(name.c_str());
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
    case TokenType::RightBrace:
      return noteParseError("Unbalanced block");
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

static inline uint8_t operatorPriority(Operator op) {
  switch (op) {
    case Operator::Equals:
    case Operator::AndEquals:
    case Operator::OrEquals:
    case Operator::MinusEquals:
    case Operator::PlusEquals:
    case Operator::StarEquals:
    case Operator::SlashEquals:
      return 1;

    case Operator::Shl:
    case Operator::Shr:
      return 2;

    case Operator::Or:
    case Operator::OrOr:
    case Operator::And:
    case Operator::AndAnd:
    case Operator::EqualsEquals:
      return 3;

    case Operator::Plus:
    case Operator::Minus:
    case Operator::Gt:
    case Operator::Lt:
    case Operator::Le:
    case Operator::Ge:
      return 4;

    case Operator::Star:
    case Operator::Slash:
      return 5;

    case Operator::PlusPlus:
    case Operator::MinusMinus:
      return 6;
  }
  assert(false && "How?");
  return 0;
}

std::unique_ptr<ast::Expression> Parser::parseExpression() {
  return parseWithOperatorPriorityAtLeast(0);
}

std::unique_ptr<ast::Expression> Parser::parseWithOperatorPriorityAtLeast(
    uint8_t minPriority) {
  auto expr = parseOneExpression();
  if (!expr)
    return nullptr;

  while (true) {
    auto tok = nextToken();
    if (!tok)
      return noteParseError(m_tokenizer.errorMessage());
    if (tok->type() != TokenType::Operator ||
        operatorPriority(tok->op()) < minPriority) {
      m_lastToken = std::move(tok);
      return expr;
    }

    auto rhs = parseWithOperatorPriorityAtLeast(operatorPriority(tok->op()));
    if (!rhs)
      return nullptr;
    expr = std::make_unique<ast::BinaryOperation>(tok->op(), std::move(expr),
                                                  std::move(rhs));
  }
}

std::unique_ptr<ast::ConditionalExpression>
Parser::tryParseRemainingConditionalBranches() {
  Optional<Token> tok = nextToken();
  if (!tok || tok->type() != TokenType::Keyword ||
      tok->keyword() != Keyword::Else) {
    m_lastToken = std::move(tok);
    return nullptr;
  }

  tok = nextToken();
  if (!tok) {
    m_lastToken = std::move(tok);
    return nullptr;
  }

  std::unique_ptr<ast::Expression> conditional;
  if (tok->type() == TokenType::Keyword && tok->keyword() == Keyword::If) {
    tok = nextToken();
    if (!tok || tok->type() != TokenType::LeftParen) {
      noteParseError("Expected parenthesis after if keyword");
      return nullptr;
    }
    conditional = parseExpression();
    if (!conditional)
      return nullptr;
    tok = nextToken();
    if (!tok || tok->type() != TokenType::RightParen) {
      noteParseError("Expected parenthesis after if condition");
      return nullptr;
    }
  } else {
    m_lastToken = std::move(tok);
  }

  std::unique_ptr<ast::Expression> body = parseExpression();
  if (!body)
    return nullptr;

  std::unique_ptr<ast::ConditionalExpression> elseBranch;
  if (conditional) {
    elseBranch = tryParseRemainingConditionalBranches();
    if (!elseBranch && m_parseError)
      return nullptr;
  }

  return std::make_unique<ast::ConditionalExpression>(
      std::move(conditional), std::move(body), std::move(elseBranch));
}
