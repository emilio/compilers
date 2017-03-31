#pragma once

#include "Parser.h"
#include "TestReader.h"

template <typename Callback>
void parse(const char* str, Callback cb) {
  TestReader reader(str);
  Tokenizer tokenizer(reader);
  Parser parser(tokenizer);
  ast::Node* result = parser.parse();
  const ParseError* error = parser.error();
  cb(result, error);
}
