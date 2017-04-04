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

#include "ASTEvaluator.h"
#include "TestUtils.h"
#include "gtest/gtest.h"

void assertExprValue(const char* expr, double val) {
  parse(expr, [val](ast::Node* node, const ParseError* error) {
    ast::ASTEvaluatorContext ctx;
    EXPECT_TRUE(toExpression(node)->evaluate(ctx).normalizedValue() == val);
  });
}

TEST(Evaluator, Basic) {
  assertExprValue("1 + 1 + 5", 7.0);
}

TEST(Evaluator, OperatorPrecedence) {
  assertExprValue("1 + 6 * 5", 31.0);
  assertExprValue("6 * 2 + 6 * 5", 42.0);
}

TEST(Evaluator, Cos) {
  assertExprValue("1 + cos(0)", 2.0);
}

TEST(Evaluator, Sin) {
  assertExprValue("1 + sin(0)", 1.0);
}

TEST(Evaluator, Abs) {
  assertExprValue("1 + abs(-1200)", 1201.0);
}

TEST(Evaluator, Sqr) {
  // TODO(emilio): Perhaps we should do approx_eq or something.
  assertExprValue("sqr(pow(10, 2))", 10.0);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
