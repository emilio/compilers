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

#include "AST.h"
#include "Tokenizer.h"

#include <memory>

class ParseError {
 public:
  ParseError(Span location, std::string&& message)
      : m_location(location), m_message(message) {}

  Span location() const { return m_location; }
  const std::string& message() const { return m_message; }

 private:
  Span m_location;
  std::string m_message;
};

class Parser {
 public:
  explicit Parser(Tokenizer& tokenizer) : m_tokenizer(tokenizer) {}

  ast::Node* parse();
  const ParseError* error() const { return m_parseError.get(); }

 private:
  Optional<Token> nextToken();

  std::unique_ptr<ast::Expression> parseOneExpression();
  std::unique_ptr<ast::Expression> parseExpression();
  std::unique_ptr<ast::ConditionalExpression>
  tryParseRemainingConditionalBranches();

  std::unique_ptr<ast::Expression> parseProduct();

  // The return value here is just convenience, it always returns null.
  std::unique_ptr<ast::Expression> noteParseError(std::string&& message);

  Tokenizer& m_tokenizer;

  // We need this lookahead token :(
  Optional<Token> m_lastToken;

  std::unique_ptr<ast::Node> m_astRoot{nullptr};
  std::unique_ptr<ParseError> m_parseError{nullptr};
};
