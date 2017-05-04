#pragma once

#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include "Bytecode.h"

class ExecutionContext {
  std::stack<Value> m_valueStack;
  std::unordered_map<LabelId, Value> m_variables;
  bool m_hasPendingError{false};
  std::string m_errorMsg;

  ExecutionContext() = default;

  /**
   * TODO(emilio): Labels and external function stuff goes here too.
   */
 public:
  static std::unique_ptr<ExecutionContext> createDefault() {
    return std::unique_ptr<ExecutionContext>(new ExecutionContext);
  }

  const Value pop() {
    assert(!m_valueStack.empty());
    assert(!m_hasPendingError);
    const Value ret = m_valueStack.top();
    m_valueStack.pop();
    return ret;
  }

  void push(Value&& val) {
    assert(!m_hasPendingError);
    m_valueStack.push(std::move(val));
  }

  const Value* stackTop() const {
    if (m_hasPendingError)
      return nullptr;

    if (m_valueStack.empty())
      return nullptr;

    return &m_valueStack.top();
  }

  void noteError(const std::string& msg) {
    m_errorMsg = msg;
    m_hasPendingError = true;
  }

  void clearVariable(LabelId);
  void setVariable(LabelId, Value);
  Value getVariable(LabelId);
};
