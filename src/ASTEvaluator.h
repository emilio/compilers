#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "Optional.h"
#include "Value.h"

namespace ast {

struct EvaluatorScope {
  std::unordered_map<std::string, Value> m_variableBindings;
};

// TODO(emilio): Track errors appropriately.
class ASTEvaluatorContext {
  std::vector<EvaluatorScope> m_scopes;

 public:
  ASTEvaluatorContext() { m_scopes.push_back(EvaluatorScope{}); }

  Optional<Value> resolveVariableBinding(const std::string&) const;
  void setVariable(const std::string&, Value);
  template <typename Callable>
  void enterScope(Callable);
};

template <typename Callable>
void ASTEvaluatorContext::enterScope(Callable fn) {
  m_scopes.push_back(EvaluatorScope{});
  fn();
  m_scopes.pop_back();
}

}  // namespace ast
