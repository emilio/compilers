#include "ASTEvaluator.h"

namespace ast {

Optional<Value> ASTEvaluatorContext::resolveVariableBinding(
    const std::string& name) const {
  assert(!m_scopes.empty());

  // Resolve variables from the innermost scope to the outermost.
  for (auto current = m_scopes.size(); current; current--) {
    auto value = m_scopes[current - 1].m_variableBindings.find(name);
    if (value != m_scopes[current - 1].m_variableBindings.end())
      return Some(value->second);
  }

  return None;
}

}  // namespace ast
