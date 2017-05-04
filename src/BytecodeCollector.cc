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

void BytecodeCollector::pushAssignTo(LabelId id) {
  m_bytecode.emplace_back(Instruction::StoreVar);
  m_bytecode.emplace_back(Bytecode::label(id));
}

void BytecodeCollector::pushLoadVar(LabelId id) {
  m_bytecode.emplace_back(Instruction::LoadVar);
  m_bytecode.emplace_back(Bytecode::label(id));
}

void BytecodeCollector::popFromStack() {
  m_bytecode.emplace_back(Instruction::Pop);
}

void BytecodeCollector::binOp(Operator op) {
  switch (op) {
    case Operator::Plus:
      return m_bytecode.emplace_back(Instruction::Add);
    case Operator::Minus:
      return m_bytecode.emplace_back(Instruction::Subtract);
    case Operator::Slash:
      return m_bytecode.emplace_back(Instruction::Div);
    case Operator::Star:
      return m_bytecode.emplace_back(Instruction::Mul);
    case Operator::Equals:
    default:
      // TODO
      break;
  }
}

void BytecodeCollector::pushScope() {
  m_scopes.push_back(Scope());
}

void BytecodeCollector::popScope() {
  assert(!m_scopes.empty());
  {
    const Scope& scope = m_scopes.back();
    for (const auto& var : scope.m_variables) {
      m_bytecode.emplace_back(Instruction::ClearVar);
      m_bytecode.push_back(Bytecode::label(var.second));
    }
  }
  m_scopes.pop_back();
}

Optional<LabelId> BytecodeCollector::resolveVariable(const std::string& name) {
  auto it = m_scopes.crbegin();
  const auto end = m_scopes.crend();
  for (; it != end; ++it) {
    const auto found = it->m_variables.find(name);
    if (found != it->m_variables.end())
      return Some(found->second);
  }

  return None;
}

LabelId BytecodeCollector::reserveVariableIdFor(const std::string& name) {
  if (auto id = resolveVariable(name))
    return *id;
  const auto id = ++m_lastVariableId;
  m_scopes.back().m_variables.emplace(name, id);
  return id;
}
