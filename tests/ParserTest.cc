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

#include "TestUtils.h"
#include "gtest/gtest.h"

void assertParses(const char* input) {
  parse(input,
        [](ast::Node* node, const ParseError* error) { EXPECT_FALSE(error); });
}

TEST(Parser, NestedBlocks) {
  assertParses("{ {} }");
}

TEST(Parser, MultipleStatementsPlusExpression) {
  assertParses("{ foo; bar; baz }");
}

TEST(Parser, MultipleStatementsPlusNestedBlock) {
  assertParses("{ 2 + 2; foo; { 2 + 3 } }");
}

TEST(Parser, NestedBlocksStatements) {
  assertParses("{ { 2 + 2 }; {}; foo; { 2 + 3 }; }");
}

TEST(Parser, IfBasic) {
  assertParses("if (foo == bar) foo();");
  assertParses("if (foo == bar) { foo() } else { bar() }");
  assertParses("if (foo == bar) { foo() } else bar()");
  // TODO(emilio): This parses r/n as:
  //
  //   if (foo == 3) { if (bar == 4) bar(); else foo(); }
  //
  // Make it unambiguous.
  assertParses("if (foo == 3) if (bar == 4) bar() else foo()");
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
