#pragma once

#include "Tokenizer.h"
#include "AST.h"

#include <memory>

class ParseError {
 public:
   ParseError(Span location, std::string&& message)
     : m_location(location)
     , m_message(message) {}

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

  std::unique_ptr<ast::Expression> parseExpression();

  // The return value here is just convenience, it always returns null.
  std::unique_ptr<ast::Expression> noteParseError(std::string&& message);

  Tokenizer& m_tokenizer;

  // We need this lookahead token :(
  Optional<Token> m_lastToken;

  std::unique_ptr<ast::Node> m_astRoot { nullptr };
  std::unique_ptr<ParseError> m_parseError { nullptr };
};
