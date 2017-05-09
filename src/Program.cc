#include "Program.h"
#include <iostream>
#include "AST.h"
#include "BytecodeCollector.h"
#include "ExecutionContext.h"
#include <cmath>

class ProgramExecutionState {
 public:
  ProgramExecutionState(const std::vector<Bytecode>& bytecode,
                        ExecutionContext& ctx)
      : m_bytecode(bytecode), m_ctx(ctx) {
    assert(!m_bytecode.empty());
  }

  bool execute();
  bool executeInstruction(Instruction);
  bool executeFunction(BuiltinFunction id, size_t args);
  bool executeAbs(size_t args);
  bool executeCos(size_t args);
  bool executePow(size_t args);
  bool executeSin(size_t args);
  bool executeSqrt(size_t args);

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
    return at(offset).value();
  }

  const LabelId expectLabelAt(ssize_t offset) const {
    return at(offset).labelId();
  }

  const size_t expectArgumentCountAt(ssize_t offset) const {
    return at(offset).argumentCount();
  }

  const BuiltinFunction expectFunctionAt(ssize_t offset) const {
    return at(offset).function();
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
      case BytecodeKind::ArgumentCount:
      case BytecodeKind::Offset:
      case BytecodeKind::LabelId:
      case BytecodeKind::BuiltinFunctionId:
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
IMPL_OP(mul, *, |)  // Dubious: do type-checking and prevent this!
IMPL_OP(div, /, &)  // Dubious: do type-checking and prevent this!

bool ProgramExecutionState::executeInstruction(Instruction ins) {
  switch (ins) {
    case Instruction::Subtract:
    case Instruction::Add:
    case Instruction::Mul:
    case Instruction::Div: {
      auto r = m_ctx.pop();
      auto l = m_ctx.pop();
      if (r.type() != l.type()) {
        // Hack for unary negation of integers.
        //
        // TODO(emilio): Either do type checking and put the correct value from
        // the bytecode generator, or create integer coercion rules.
        if (l.type() == ValueType::Float &&
            l.doubleValue() == 0. &&
            r.type() == ValueType::Integer) {
          l = Value::createInt(0);
        } else {
          return error("Mismatched types in binary operation");
        }
      }
      switch (ins) {
        case Instruction::Subtract:
          m_ctx.push(subractValues(l, r));
          break;
        case Instruction::Add:
          m_ctx.push(addValues(l, r));
          break;
        case Instruction::Mul:
          m_ctx.push(mulValues(l, r));
          break;
        case Instruction::Div:
          m_ctx.push(divValues(l, r));
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
    case Instruction::StoreVar: {
      assert(m_ctx.stackTop());
      Value val = *m_ctx.stackTop();
      LabelId id = expectLabelAt(1);
      m_ctx.setVariable(id, val);
      advance(2);
      return true;
    }
    case Instruction::ClearVar: {
      LabelId id = expectLabelAt(1);
      m_ctx.clearVariable(id);
      advance(2);
      return true;
    }
    case Instruction::LoadVar: {
      LabelId id = expectLabelAt(1);
      Value val = m_ctx.getVariable(id);
      m_ctx.push(std::move(val));
      advance(2);
      return true;
    }
    case Instruction::CallFunction: {
      BuiltinFunction id = expectFunctionAt(1);
      size_t args = expectArgumentCountAt(2);
      if (!executeFunction(id, args))
        return error("Error in function evaluation");
      advance(3);
      return true;
    }

    default:
      std::cerr << "Found unknown (yet) instruction: " << ins << std::endl;
      return false;
  }
  return false;
}

bool ProgramExecutionState::executeFunction(BuiltinFunction id, size_t args) {
  switch (id) {
    case BuiltinFunction::Abs:
      return executeAbs(args);
    case BuiltinFunction::Pow:
      return executePow(args);
    case BuiltinFunction::Cos:
      return executeCos(args);
    case BuiltinFunction::Sin:
      return executeSin(args);
    case BuiltinFunction::Sqrt:
      return executeSqrt(args);
  }
  assert(false && "unknown function!");
  return false;
}

template<typename IntFunction, typename DoubleFunction>
static bool simpleIntFunction(ExecutionContext& ctx,
                              size_t argCount,
                              bool intReturnsDouble,
                              IntFunction intFn,
                              DoubleFunction doubleFn) {
  if (argCount != 1)
    return false;
  Value val = ctx.pop();
  switch (val.type()) {
    case ValueType::Bool:
      return false;
    case ValueType::Float:
      ctx.push(Value::createDouble(doubleFn(val.doubleValue())));
      return true;
    case ValueType::Integer:
      if (intReturnsDouble)
        ctx.push(Value::createDouble(intFn(val.intValue())));
      else
        ctx.push(Value::createInt(intFn(val.intValue())));
      return true;
  }

  assert(false && "Invalid value!");
  return false;
}

bool ProgramExecutionState::executeAbs(size_t args) {
  return simpleIntFunction(m_ctx, args, false, abs, fabs);
}

bool ProgramExecutionState::executeCos(size_t args) {
  return simpleIntFunction(m_ctx, args, true, cos, cos);
}

bool ProgramExecutionState::executeSin(size_t args) {
  return simpleIntFunction(m_ctx, args, true, sin, sin);
}

bool ProgramExecutionState::executeSqrt(size_t args) {
  return simpleIntFunction(m_ctx, args, true, sqrt, sqrt);
}

bool ProgramExecutionState::executePow(size_t args) {
  if (args != 2)
    return false;

  Value lhs = m_ctx.pop();
  Value rhs = m_ctx.pop();

  if (lhs.type() != rhs.type())
    return false;

  switch (lhs.type()) {
    case ValueType::Bool:
      return false;
    case ValueType::Integer:
      m_ctx.push(Value::createInt(std::pow(lhs.intValue(), rhs.intValue())));
      return true;
    case ValueType::Float:
      m_ctx.push(Value::createDouble(std::pow(lhs.doubleValue(), rhs.doubleValue())));
      return true;
  }

  assert(false && "Invalid value!");
  return false;
}
