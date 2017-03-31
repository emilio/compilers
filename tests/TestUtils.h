#pragma once

#include "TestReader.h"
#include "Parser.h"

template<typename Callback>
void parse(const char* str, Callback cb) {
  TestReader reader(str);
  Tokenizer tokenizer(reader);
  Parser parser(tokenizer);
  ast::Node* result = parser.parse();
  const ParseError* error = parser.error();
  cb(result, error);
}
