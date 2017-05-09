#include "ExecutionContext.h"

void ExecutionContext::setVariable(LabelId id, Value val) {
  m_variables.insert_or_assign(id, std::move(val));
}

void ExecutionContext::clearVariable(LabelId id) {
  assert(m_variables.find(id) != m_variables.end());
  m_variables.erase(id);
}

Value ExecutionContext::getVariable(LabelId id) {
  assert(m_variables.find(id) != m_variables.end());
  return m_variables.find(id)->second;
}

std::ostream& operator<<(std::ostream& os, const ExecutionContext& ctx) {
  os << "ExecutionContext(\n";
  os << "  Vars(\n";
  for (const auto& var : ctx.m_variables)
    os << "    " << var.first << ": " << var.second << "\n";
  os << "  )\n";
  os << "  Stack(\n";
  std::stack<Value> copy = ctx.m_valueStack;
  while (!copy.empty()) {
    os << "    " << copy.top() << "\n";
    copy.pop();
  }
  os << "  )\n";
  os << ")";

  return os;
}
