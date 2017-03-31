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

static bool isOperator(char which) {
  return which == '+' || which == '*' || which == '-' || which == '/';
}

static bool isTokenSeparator(char which) {
  return isWhitespace(which) || isOperator(which) || which == '(' ||
         which == ')' || !which;
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

  if (isOperator(next))
    return Some(Token::createOp(next, location));

  if (next == ')')
    return Some(Token::createOfType(TokenType::RightParen, location));

  if (next == '(')
    return Some(Token::createOfType(TokenType::LeftParen, location));

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

    return Some(Token::createIdent(ident.c_str(), location));
  }

  return error("unknown token");
}
