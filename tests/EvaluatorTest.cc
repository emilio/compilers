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

#include "ExecutionContext.h"
#include "Program.h"
#include "TestUtils.h"
#include "gtest/gtest.h"

void assertExprValue(const char* expr, const Value& val) {
  parse(expr, [&](ast::Node* node, const ParseError* error) {
    EXPECT_TRUE(node);
    auto programResult = Program::fromAST(*node);
    EXPECT_TRUE(programResult);
    auto program = programResult.unwrap();
    std::unique_ptr<ExecutionContext> ctx = ExecutionContext::createDefault();
    bool result = program->execute(*ctx);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ctx->stackTop());
    EXPECT_EQ(val, *ctx->stackTop());
  });
}

TEST(Evaluator, Basic) {
  assertExprValue("1 + 1 + 5", Value::createInt(7));
}

TEST(Evaluator, OperatorPrecedence) {
  assertExprValue("1 + 6 * 5", Value::createInt(31));
  assertExprValue("6 * 2 + 6 * 5", Value::createInt(42));
}

TEST(Evaluator, SimpleVarsAndPrecedence) {
  const char* kProgram =
      "{"
      "a = 15;"
      "b = 10;"
      "a = a + b;"
      "a + a + a"
      "}";

  assertExprValue(kProgram, Value::createInt(75));
}

// TEST(Evaluator, Cos) {
//   assertExprValue("1 + cos(0)", Value::createDouble(2.0));
// }
//
// TEST(Evaluator, Sin) {
//   assertExprValue("1 + sin(0)", Value::createDouble(1.0));
// }
//
// TEST(Evaluator, Abs) {
//   assertExprValue("1 + abs(-1200)", Value::createInt(1201));
// }
//
// TEST(Evaluator, Sqr) {
//   // TODO(emilio): Perhaps we should do approx_eq or something.
//   assertExprValue("sqr(pow(10, 2))", Value::createDouble(10.0));
// }

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
