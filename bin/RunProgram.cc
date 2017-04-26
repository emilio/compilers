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

#include <iostream>

#include "AST.h"
#include "ExecutionContext.h"
#include "FileReader.h"
#include "Parser.h"
#include "Program.h"
#include "Tokenizer.h"

int main(int argc, const char** argv) {
  if (argc < 2) {
    std::cerr << "Need at least a filename.\n";
    return 1;
  }
  FileReader reader(argv[1]);
  Tokenizer tokenizer(reader);
  Parser parser(tokenizer);

  ast::Node* node = parser.parse();
  if (!node) {
    const ParseError* error = parser.error();
    std::cerr << "parse error @ " << error->location() << ": "
              << error->message() << std::endl;
    return 1;
  }

  auto programResult = Program::fromAST(*node);
  if (!programResult) {
    // FIXME(emilio): Meaningful error!
    std::cerr << "Couldn't create program: "
              << programResult.unwrapErr().message() << std::endl;
    return 1;
  }

  auto program = programResult.unwrap();
  assert(program);

  std::cout << *program << std::endl;

  std::unique_ptr<ExecutionContext> ctx = ExecutionContext::createDefault();
  assert(ctx);
  if (!program->execute(*ctx)) {
    // FIXME(emilio): Meaningful error!
    std::cerr << "program evaluation failed" << std::endl;
    return 1;
  }

  // TODO(emilio): Perhaps a context dump would be nicer.
  if (const Value* val = ctx->stackTop())
    std::cout << *val << std::endl;
  else
    std::cout << "<unit>" << std::endl;

  return 0;
}
