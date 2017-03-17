#include "Parser.h"

#include <memory>

ast::Node*
Parser::parse() {
  m_astRoot = parseExpression();

  auto tok = m_tokenizer.nextToken();
  if (!tok || tok->type() != TokenType::Eof)
    m_astRoot.reset();

  if (!m_astRoot)
    assert(m_parseError);
  return m_astRoot.get();
}

std::unique_ptr<ast::Expression>
Parser::noteParseError(std::string&& message) {
  assert(!m_parseError);
  assert(!m_astRoot);
  m_parseError.reset(new ParseError(m_tokenizer.location(), std::move(message)));
  return nullptr;
}

Optional<Token>
Parser::nextToken() {
  if (m_lastToken)
    return std::move(m_lastToken);
  return m_tokenizer.nextToken();
}

std::unique_ptr<ast::Expression>
Parser::parseExpression() {
  Optional<Token> tok = nextToken();

  switch (tok->type()) {
    case TokenType::Float:
    case TokenType::Number: {
      // Maybe it's a standalone token, maybe it's the lhs of an arbitrarily
      // long binary expression tree.
      Value val = tok->type() == TokenType::Number
        ? Value::createInt(tok->number())
        : Value::createDouble(tok->doubleValue());

      auto maybe_lhs = std::make_unique<ast::ConstantExpression>(val);
      if (!maybe_lhs)
        return nullptr;

      Optional<Token> tok = nextToken();
      if (!tok)
        return noteParseError(m_tokenizer.errorMessage());
      if (tok->type() != TokenType::Operator) {
        m_lastToken = std::move(tok);
        return maybe_lhs;
      }

      // TODO(emilio): This recursiveness is quite nice, but it doesn't take
      // operator precedence into account, does it?
      auto rhs = parseExpression();
      if (!rhs)
        return nullptr;

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
      if (!tok)
        return noteParseError(m_tokenizer.errorMessage());

      // We only support function calls, so that makes it a bit easier, we could
      // also set m_lastToken to tok here and return a variable binding if
      // needed.
      if (tok->type() != TokenType::LeftParen)
        return noteParseError("Expected opening parenthesis for function call");

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
      char op = tok->op();
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
