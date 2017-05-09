#pragma once

#include "Bytecode.h"
#include "Tokenizer.h"

#include <unordered_map>
#include <vector>

class BytecodeCollector {
  struct Scope {
    std::unordered_map<std::string, LabelId> m_variables;
  };

  std::vector<Bytecode> m_bytecode;
  std::vector<Scope> m_scopes;
  LabelId m_lastVariableId{0};

 public:
  BytecodeCollector() : m_lastVariableId(0) { m_scopes.push_back(Scope()); }

  std::vector<Bytecode> takeBytecode();

  void pushToStack(Value);
  void popFromStack();

  void pushScope();
  void popScope();

  void pushAssignTo(LabelId);
  void pushLoadVar(LabelId);
  void binOp(Operator);

  Optional<LabelId> resolveVariable(const std::string& name);
  LabelId reserveVariableIdFor(const std::string& name);
};
