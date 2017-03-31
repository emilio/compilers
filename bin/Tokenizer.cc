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
