#include <cstdio>
#include <iostream>
#include "FileReader.h"
#include "Parser.h"
#include "Tokenizer.h"

// TODO(emilio): This should probably become a proper unit test with gtest or
// something like that.

int main(int, const char**) {
  FileReader reader(stdin, false);
  Tokenizer tokenizer(reader);
  Parser parser(tokenizer);

  if (ast::Node* node = parser.parse()) {
    if (isExpression(node)) {
      Value val = toExpression(node)->evaluate();
      std::cout << val.normalizedValue() << std::endl;
    }
  } else if (const ParseError* error = parser.error()) {
    std::cout << "parse error @ " << error->location() << ": "
              << error->message() << std::endl;
  } else {
    assert(false && "How!");
  }
}
