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

#include "Tokenizer.h"
#include <string>

static bool isWhitespace(char which) {
  // Well, unicode people won't love it, but for a simple school assignment,
  // this can be enough.
  return which == '\n' || which == '\t' || which == ' ';
}

static bool isNumeric(char which) {
  return which >= '0' && which <= '9';
}

static bool isAlphabetic(char which) {
  return (which >= 'a' && which <= 'z') || (which >= 'A' && which <= 'Z');
}

static bool isIdentifierStart(char which) {
  return which == '_' || isAlphabetic(which);
}

static bool isIdentPart(char which) {
  return isIdentifierStart(which) || isNumeric(which);
}

template <typename... Args>
bool isAnyOf(char which, char one, Args... args) {
  return which == one || isAnyOf(which, args...);
}

template <>
bool isAnyOf(char which, char one) {
  return which == one;
}

static Optional<Operator> toOperator(char first,
                                     char second,
                                     bool& consumedSecond) {
  consumedSecond = false;
#define SIMPLE_CASE(op_, single_, double_, equals_) \
  case op_:                                         \
    if (second == op_) {                            \
      consumedSecond = true;                        \
      return Some(Operator::double_);               \
    }                                               \
    if (second == '=') {                            \
      consumedSecond = true;                        \
      return Some(Operator::equals_);               \
    }                                               \
    return Some(Operator::single_);

#define SIMPLE_CASE2(op_, single_, equals_) \
  case op_:                                 \
    if (second == '=') {                    \
      consumedSecond = true;                \
      return Some(Operator::equals_);       \
    }                                       \
    return Some(Operator::single_);

  switch (first) {
    SIMPLE_CASE('+', Plus, PlusPlus, PlusEquals)
    SIMPLE_CASE('-', Minus, MinusMinus, MinusEquals)
    SIMPLE_CASE('<', Lt, Shl, Le)
    SIMPLE_CASE('>', Gt, Shr, Ge)
    SIMPLE_CASE('&', And, AndAnd, AndEquals)
    SIMPLE_CASE('|', Or, OrOr, OrEquals)
    SIMPLE_CASE2('*', Star, StarEquals)
    SIMPLE_CASE2('/', Slash, SlashEquals)
    SIMPLE_CASE2('=', Equals, EqualsEquals)
    default:
      return None;
  }
}

static bool isSingleCharOperator(char which) {
  bool unused;
  return bool(toOperator(which, '\0', unused));
}

static bool isTokenSeparator(char which) {
  return isWhitespace(which) || isSingleCharOperator(which) ||
         isAnyOf(which, '(', ',', ')', '{', '}', ';', '\0');
}

static Optional<Keyword> isKeyword(const std::string& which) {
  if (which == "for")
    return Some(Keyword::For);
  if (which == "while")
    return Some(Keyword::While);
  if (which == "if")
    return Some(Keyword::If);
  if (which == "else")
    return Some(Keyword::Else);
  return None;
}

char Tokenizer::peekChar() {
  if (!m_lastChar)
    m_lastChar.set(m_reader.next());
  return *m_lastChar;
}

char Tokenizer::nextChar() {
  char which = peekChar();
  m_lastChar.clear();
  if (which)
    m_location.column += 1;
  if (which == '\n') {
    m_location.column = 0;
    m_location.line += 1;
  }
  return which;
}

Optional<Token> Tokenizer::nextToken() {
  if (m_error)
    return None;
  return nextTokenInternal();
}

Optional<Token> Tokenizer::nextTokenInternal() {
again:
  Span location = m_location;
  char next = nextChar();
  if (!next)
    return Some(Token::createOfType(TokenType::Eof, location));

  if (isWhitespace(next))
    goto again;

  bool consumedSecond;
  if (Optional<Operator> op = toOperator(next, peekChar(), consumedSecond)) {
    if (consumedSecond)
      nextChar();
    return Some(Token::createOp(*op, location));
  }

  if (next == ';')
    return Some(Token::createOfType(TokenType::SemiColon, location));

  if (next == ')')
    return Some(Token::createOfType(TokenType::RightParen, location));

  if (next == '(')
    return Some(Token::createOfType(TokenType::LeftParen, location));

  if (next == '}')
    return Some(Token::createOfType(TokenType::RightBrace, location));

  if (next == '{')
    return Some(Token::createOfType(TokenType::LeftBrace, location));

  if (next == ',')
    return Some(Token::createOfType(TokenType::Comma, location));

  if (isNumeric(next)) {
    std::string number;
    number.push_back(next);
    // TODO(emilio): We could look for hexadecimal bases and similar here, but
    // meh.
    while (isNumeric(peekChar()))
      number.push_back(nextChar());
    if (peekChar() == '.') {
      number.push_back(nextChar());
      while (isNumeric(peekChar()))
        number.push_back(nextChar());
      if (!isTokenSeparator(peekChar()))
        return error("Invalid token separator after floating point number");
      if (number[number.size() - 1] == '.')
        number.push_back('0');
      return Some(
          Token::createFloat(std::strtod(number.c_str(), nullptr), location));
    }
    if (!isTokenSeparator(peekChar()))
      return error("Invalid token separator after number");
    return Some(Token::createNumber(std::stoull(number), location));
  }

  if (isIdentifierStart(next)) {
    std::string ident;
    ident.push_back(next);

    while (isIdentPart(peekChar()))
      ident.push_back(nextChar());

    if (!isTokenSeparator(peekChar()))
      return error("Invalid token separator after identifier");

    if (Optional<Keyword> keyword = isKeyword(ident))
      return Some(Token::createKeyword(*keyword, location));

    return Some(Token::createIdent(ident.c_str(), location));
  }

  return error("unknown token");
}
