#include "TestUtils.h"
#include "gtest/gtest.h"

void assertExprValue(const char* expr, double val) {
  parse(expr, [val](ast::Node* node, const ParseError* error) {
    EXPECT_TRUE(toExpression(node)->evaluate().normalizedValue() == val);
  });
}

TEST(Evaluator, Basic) {
  assertExprValue("1 + 1 + 5", 7.0);
}

TEST(Evaluator, OperatorPrecedence) {
  assertExprValue("1 + 6 * 5", 31.0);
  assertExprValue("6 * 2 + 6 * 5", 42.0);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
