#pragma once

#include <cassert>
#include <cstring>
#include <vector>
#include <ostream>

enum class TokenType : unsigned char {
  Number,
  Identifier,
  Operator,
  LeftParen,
  RightParen,
  Eof,
};

inline std::ostream& operator<<(std::ostream& os, TokenType type) {
  switch (type) {
    case TokenType::Number:
      return os << "Number";
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

  Span(std::size_t line, std::size_t column)
   : line(line), column(column) {}

  Span() : Span(0, 0) {};
};

inline std::ostream& operator<<(std::ostream& os, const Span& span) {
  return os << "Span(" << span.line << ", " << span.column << ")";
}

class Token {
  TokenType m_type;
  Span m_span;
  union {
    char* m_ident;
    unsigned m_number;
    char m_op;
  } m_value;

  explicit Token(TokenType type, Span span) : m_type(type), m_span(span) {};

 public:
  Token(const Token&) = delete;
  Token() = delete;
  Token(Token&& other) {
    m_type = other.m_type;
    m_value = other.m_value;
    if (m_type == TokenType::Identifier)
      other.m_value.m_ident = nullptr;
  }

  ~Token() {
    if (type() == TokenType::Identifier && m_value.m_ident)
      free(m_value.m_ident);
  }

  static Token createOp(char op, Span location) {
    Token tok(TokenType::Operator, location);
    tok.m_value.m_op = op;
    return tok;
  }

  static Token createNumber(unsigned num, Span location) {
    Token tok(TokenType::Number, location);
    tok.m_value.m_number = num;
    return tok;
  }

  static Token createOfType(TokenType type, Span span) {
    assert(type != TokenType::Number &&
           type != TokenType::Operator &&
           type != TokenType::Identifier);
    return Token(type, span);
  }

  static Token createIdent(const char* string,
                           std::size_t length,
                           Span span) {
    Token tok(TokenType::Identifier, span);
    tok.m_value.m_ident = static_cast<char*>(malloc(length + 1));
    memcpy(tok.m_value.m_ident, string, length);
    tok.m_value.m_ident[length] = '\0';
    return tok;
  }

  static Token createIdent(const char* string, Span span) {
    return createIdent(string, strlen(string), span);
  }

  TokenType type() const { return m_type; }
  const Span& span() const { return m_span; }

  unsigned number() const {
    assert(type() == TokenType::Number);
    return m_value.m_number;
  }

  char op() const {
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
  Reader& m_reader;
  Span m_location;
  std::vector<char> m_savedChars;
  char peekChar();
  char nextChar();
 public:
  Tokenizer(Reader& reader) : m_reader(reader) {};
  Token nextToken();
};
