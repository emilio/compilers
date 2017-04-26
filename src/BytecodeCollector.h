#pragma once

#include "Bytecode.h"
#include "Tokenizer.h"

#include <vector>

class BytecodeCollector {
  std::vector<Bytecode> m_bytecode;

 public:
  BytecodeCollector() = default;

  std::vector<Bytecode> takeBytecode();
  void pushToStack(Value);
  void popFromStack();
  void binOp(Operator);
};
