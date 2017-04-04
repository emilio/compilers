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
#include <cstring>
#include <ostream>
#include <vector>

#include "Optional.h"

enum class Operator : unsigned char {
  Plus,
  PlusPlus,
  PlusEquals,
  Minus,
  MinusMinus,
  MinusEquals,
  Star,
  StarEquals,
  Slash,
  SlashEquals,
  Equals,
  EqualsEquals,
  And,
  AndAnd,
  AndEquals,
  Or,
  OrOr,
  OrEquals,
  Lt,
  Shl,
  Le,
  Gt,
  Shr,
  Ge,
};

inline std::ostream& operator<<(std::ostream& os, const Operator& op) {
  switch (op) {
    case Operator::Plus:
      return os << "Plus";
    case Operator::PlusPlus:
      return os << "PlusPlus";
    case Operator::PlusEquals:
      return os << "PlusEquals";
    case Operator::Minus:
      return os << "Minus";
    case Operator::MinusMinus:
      return os << "MinusMinus";
    case Operator::MinusEquals:
      return os << "MinusEquals";
    case Operator::Star:
      return os << "Star";
    case Operator::StarEquals:
      return os << "StarEquals";
    case Operator::Slash:
      return os << "Slash";
    case Operator::SlashEquals:
      return os << "SlashEquals";
    case Operator::Equals:
      return os << "Equals";
    case Operator::EqualsEquals:
      return os << "EqualsEquals";
    case Operator::And:
      return os << "And";
    case Operator::AndAnd:
      return os << "AndAnd";
    case Operator::AndEquals:
      return os << "AndEquals";
    case Operator::Or:
      return os << "Or";
    case Operator::OrOr:
      return os << "OrOr";
    case Operator::OrEquals:
      return os << "OrEquals";
    case Operator::Lt:
      return os << "Lt";
    case Operator::Le:
      return os << "Le";
    case Operator::Shl:
      return os << "Shl";
    case Operator::Gt:
      return os << "Gt";
    case Operator::Ge:
      return os << "Ge";
    case Operator::Shr:
      return os << "Shr";
  }

  assert(false);
  return os;
}

enum class TokenType : unsigned char {
  Comma,
  Number,
  Float,
  Identifier,
  Keyword,
  Operator,
  LeftParen,
  RightParen,
  SemiColon,
  Eof,
};

inline std::ostream& operator<<(std::ostream& os, TokenType type) {
  switch (type) {
    case TokenType::Keyword:
      return os << "Keyword";
    case TokenType::SemiColon:
      return os << "SemiColon";
    case TokenType::Comma:
      return os << "Comma";
    case TokenType::Number:
      return os << "Number";
    case TokenType::Float:
      return os << "Float";
    case TokenType::Identifier:
      return os << "Identifier";
    case TokenType::Operator:
      return os << "Operator";
    case TokenType::LeftParen:
      return os << "LeftParen";
    case TokenType::RightParen:
      return os << "RightParen";
    case TokenType::Eof:
      return os << "Eof";
  }
  assert(false && "Shouldn't be reached");
  return os;
}

struct Span {
  std::size_t line;
  std::size_t column;

  Span(std::size_t line, std::size_t column) : line(line), column(column) {}
  Span() : Span(0, 0) {}
};

inline std::ostream& operator<<(std::ostream& os, const Span& span) {
  return os << "Span(" << span.line << ", " << span.column << ")";
}

inline bool typeHoldsIdentifier(TokenType type) {
  return type == TokenType::Identifier || type == TokenType::Keyword;
}

class Token {
  TokenType m_type;
  Span m_span;
  union {
    char* m_ident;
    unsigned m_number;
    double m_float;
    Operator m_op;
  } m_value;

  explicit Token(TokenType type, Span span) : m_type(type), m_span(span){};

  bool typeHoldsIdentifier() const { return ::typeHoldsIdentifier(type()); }

  static Token createIdentHolderOfType(TokenType type,
                                       const char* string,
                                       std::size_t length,
                                       Span span) {
    assert(::typeHoldsIdentifier(type));
    Token tok(type, span);
    tok.m_value.m_ident = static_cast<char*>(malloc(length + 1));
    memcpy(tok.m_value.m_ident, string, length);
    tok.m_value.m_ident[length] = '\0';
    return tok;
  }

 public:
  Token& operator=(const Token& other) {
    m_type = other.m_type;
    m_span = other.m_span;
    m_value = other.m_value;
    if (typeHoldsIdentifier())
      m_value.m_ident = strdup(other.m_value.m_ident);
    return *this;
  }

  Token(Token&& other) {
    m_type = other.m_type;
    m_value = other.m_value;
    m_span = other.m_span;
    if (typeHoldsIdentifier())
      other.m_value.m_ident = nullptr;
  }

  ~Token() {
    if (typeHoldsIdentifier() && m_value.m_ident)
      free(m_value.m_ident);
  }

  static Token createOp(Operator op, Span location) {
    Token tok(TokenType::Operator, location);
    tok.m_value.m_op = op;
    return tok;
  }

  static Token createNumber(unsigned num, Span location) {
    Token tok(TokenType::Number, location);
    tok.m_value.m_number = num;
    return tok;
  }

  static Token createFloat(double num, Span location) {
    Token tok(TokenType::Float, location);
    tok.m_value.m_float = num;
    return tok;
  }

  static Token createOfType(TokenType type, Span span) {
    assert(type != TokenType::Number && type != TokenType::Float &&
           type != TokenType::Operator && type != TokenType::Identifier &&
           type != TokenType::Keyword);
    return Token(type, span);
  }

  static Token createIdent(const char* string, std::size_t length, Span span) {
    return createIdentHolderOfType(TokenType::Identifier, string, length, span);
  }

  static Token createIdent(const std::string& str, Span span) {
    return createIdent(str.c_str(), str.length(), span);
  }

  static Token createIdent(const char* string, Span span) {
    return createIdent(string, strlen(string), span);
  }

  static Token createKeyword(const char* string,
                             std::size_t length,
                             Span span) {
    return createIdentHolderOfType(TokenType::Keyword, string, length, span);
  }

  static Token createKeyword(const std::string& str, Span span) {
    return createKeyword(str.c_str(), str.length(), span);
  }

  static Token createKeyword(const char* string, Span span) {
    return createKeyword(string, strlen(string), span);
  }

  TokenType type() const { return m_type; }
  const Span& span() const { return m_span; }

  unsigned number() const {
    assert(type() == TokenType::Number);
    return m_value.m_number;
  }

  double doubleValue() const {
    assert(type() == TokenType::Float);
    return m_value.m_float;
  }

  Operator op() const {
    assert(type() == TokenType::Operator);
    return m_value.m_op;
  }

  const char* ident() const {
    assert(type() == TokenType::Identifier);
    return m_value.m_ident;
  }
};

inline std::ostream& operator<<(std::ostream& os, const Token& token) {
  os << "Token(" << token.type() << " @ " << token.span();
  switch (token.type()) {
    case TokenType::Number:
      os << ", " << token.number();
      break;
    case TokenType::Float:
      os << ", " << token.doubleValue();
      break;
    case TokenType::Operator:
      os << ", " << token.op();
      break;
    case TokenType::Identifier:
      os << ", \"" << token.ident() << "\"";
      break;
    default:
      break;
  }
  return os << ")";
}

// Reader must provide a method `next()`, that returns a `char`, or `'\0'` at
// EOF.
//
// FIXME(emilio): This is a quite shitty abstraction.
class Reader {
 public:
  virtual ~Reader() = default;
  virtual char next() = 0;
};

class Tokenizer {
 public:
  Span location() const { return m_location; }
  const char* errorMessage() const { return m_error; }
  Tokenizer(Reader& reader) : m_reader(reader){};
  Optional<Token> nextToken();

 private:
  Optional<Token> nextTokenInternal();
  Optional<Token> error(const char* message) {
    m_error = message;
    return None;
  }
  char peekChar();
  char nextChar();

  Reader& m_reader;
  Span m_location;
  const char* m_error{nullptr};
  Optional<char> m_lastChar;
};
