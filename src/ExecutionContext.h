#pragma once

#include <memory>
#include <stack>
#include "Bytecode.h"

class ExecutionContext {
  std::stack<Value> m_valueStack;
  bool m_hasPendingError{false};

  ExecutionContext() = default;

  /**
   * TODO(emilio): Labels and external function stuff goes here.
   */
 public:
  static std::unique_ptr<ExecutionContext> createDefault() {
    return std::unique_ptr<ExecutionContext>(new ExecutionContext);
  }

  const Value* stackTop() const {
    if (m_hasPendingError)
      return nullptr;

    if (m_valueStack.empty())
      return nullptr;

    return &m_valueStack.top();
  }
};
