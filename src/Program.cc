#include "Program.h"
#include "AST.h"
#include "BytecodeCollector.h"
#include "ExecutionContext.h"

class ProgramExecutionState {
 public:
  ProgramExecutionState(const std::vector<Bytecode>& bytecode,
                        ExecutionContext& ctx)
      : m_bytecode(bytecode), m_ctx(ctx) {
    assert(!m_bytecode.empty());
  }

  bool execute();
  bool executeInstruction(Instruction);
  const Bytecode& at(ssize_t offset) const {
    size_t final_offset = m_pc + offset;
    assert(final_offset < m_bytecode.size());
    return m_bytecode[final_offset];
  }

  bool done() const { return m_pc >= m_bytecode.size(); }

  void jmp(ssize_t offset) {
    if (offset < 0) {
      assert(m_pc >= static_cast<size_t>(-offset));
    }
    m_pc += offset;
  }

  void advance(size_t offset) { jmp(offset); }

  const Bytecode& curr() const { return at(0); }

  const Value& expectValueAt(ssize_t offset) const {
    const auto& bc = at(offset);
    assert(bc.kind() == BytecodeKind::Value);
    return bc.value();
  }

  bool error(const std::string& msg) {
    m_ctx.noteError(msg);
    return false;
  }

 private:
  const std::vector<Bytecode>& m_bytecode;
  ExecutionContext& m_ctx;
  size_t m_pc{0};
};

Result<std::unique_ptr<Program>, ProgramCreationError> Program::fromAST(
    const ast::Node& ast) {
  BytecodeCollector collector;
  ast::BytecodeCollectionResult result = ast.toByteCode(collector);
  if (!result)
    return ProgramCreationError(result.unwrapErr());
  return std::unique_ptr<Program>(new Program(collector.takeBytecode()));
}

bool Program::execute(ExecutionContext& ctx) {
  ProgramExecutionState state(m_bytecode, ctx);
  return state.execute();
}

std::ostream& operator<<(std::ostream& os, const Program& program) {
  os << "Program(\n";
  for (const auto& bytecode : program.m_bytecode)
    os << "  " << bytecode << '\n';
  return os << ")";
}

bool ProgramExecutionState::execute() {
  while (!done()) {
    const Bytecode& current = curr();
    switch (current.kind()) {
      case BytecodeKind::Value:
      case BytecodeKind::LabelId:
      case BytecodeKind::ExternalFunctionId:
        assert(false &&
               "unexpected top-level bytecode kind, expected an instruction");
        return false;
      case BytecodeKind::Instruction:
        if (!executeInstruction(current.instruction()))
          return false;
        break;
    }
  }
  return true;
}

#define IMPL_OP(name_, op_, boolop_)                                     \
  static Value name_##Values(const Value& l, const Value& r) {           \
    assert(r.type() == l.type());                                        \
                                                                         \
    switch (l.type()) {                                                  \
      case ValueType::Integer:                                           \
        return Value::createInt(l.intValue() op_ r.intValue());          \
      case ValueType::Float:                                             \
        return Value::createDouble(l.doubleValue() op_ r.doubleValue()); \
      case ValueType::Bool:                                              \
        bool val = l.boolValue() boolop_ r.boolValue();                  \
        return Value::createBool(val);                                   \
    }                                                                    \
    __builtin_unreachable();                                             \
  }

IMPL_OP(add, +, ||)
IMPL_OP(subract, -, -)

bool ProgramExecutionState::executeInstruction(Instruction ins) {
  switch (ins) {
    case Instruction::Subtract:
    case Instruction::Add: {
      auto l = m_ctx.pop();
      auto r = m_ctx.pop();
      if (r.type() != l.type())
        return error("Mismatched types in add");
      switch (ins) {
        case Instruction::Subtract:
          m_ctx.push(subractValues(l, r));
          break;
        case Instruction::Add:
          m_ctx.push(addValues(l, r));
          break;
        default:
          __builtin_unreachable();
      }
      advance(1);
      return true;
    }
    case Instruction::Load: {
      Value val = expectValueAt(1);
      m_ctx.push(std::move(val));
      advance(2);
      return true;
    }
    case Instruction::Pop: {
      m_ctx.pop();
      advance(1);
      return true;
    }
    default:
      return false;
  }
  return false;
}
