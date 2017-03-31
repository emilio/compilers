#include "Tokenizer.h"
#include <cstdio>
#include <iostream>
#include "FileReader.h"

// TODO(emilio): This should probably become a proper unit test with gtest or
// something like that.

int main(int, const char**) {
  FileReader reader(stdin, false);
  Tokenizer tokenizer(reader);

  while (true) {
    Optional<Token> token = tokenizer.nextToken();
    if (!token) {
      std::cout << "Tokenizer error: " << tokenizer.errorMessage() << " @ "
                << tokenizer.location() << std::endl;
      break;
    }
    std::cout << *token << std::endl;
    if (token->type() == TokenType::Eof)
      break;
  }
}
