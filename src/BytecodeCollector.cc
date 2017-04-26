#include "BytecodeCollector.h"

#include "Bytecode.h"

#include <memory>
#include <vector>

std::vector<Bytecode> BytecodeCollector::takeBytecode() {
  return std::move(m_bytecode);
}

void BytecodeCollector::pushToStack(Value val) {
  m_bytecode.emplace_back(Instruction::Load);
  m_bytecode.emplace_back(std::move(val));
}

void BytecodeCollector::popFromStack() {
  m_bytecode.emplace_back(Instruction::Pop);
}

void BytecodeCollector::binOp(Operator op) {
  switch (op) {
    case Operator::Plus:
      return m_bytecode.emplace_back(Instruction::Add);
    default:
      // TODO
      break;
  }
}
