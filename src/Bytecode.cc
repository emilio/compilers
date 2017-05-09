#include "Bytecode.h"

std::ostream& operator<<(std::ostream& os, const Instruction& kind) {
  switch (kind) {
    case Instruction::Div:
      return os << "Div";
    case Instruction::Mul:
      return os << "Mul";
    case Instruction::Add:
      return os << "Add";
    case Instruction::Load:
      return os << "Load";
    case Instruction::Pop:
      return os << "Pop";
    case Instruction::LoadVar:
      return os << "LoadVar";
    case Instruction::Subtract:
      return os << "Subtract";
    case Instruction::StoreVar:
      return os << "StoreVar";
    case Instruction::CallFunction:
      return os << "CallFunction";
    case Instruction::Jump:
      return os << "Jump";
    case Instruction::JumpIfZero:
      return os << "JumpIfZero";
    case Instruction::ClearVar:
      return os << "ClearVar";
  }

  assert(false);
  return os;
}

std::ostream& operator<<(std::ostream& os, const BytecodeKind& kind) {
  switch (kind) {
    case BytecodeKind::ArgumentCount:
      return os << "ArgumentCount";
    case BytecodeKind::LabelId:
      return os << "LabelId";
    case BytecodeKind::BuiltinFunctionId:
      return os << "BuiltinFunctionId";
    case BytecodeKind::Value:
      return os << "Value";
    case BytecodeKind::Instruction:
      return os << "Instruction";
    case BytecodeKind::Offset:
      return os << "Offset";
  }

  assert(false);
  return os;
}

std::ostream& operator<<(std::ostream& os, const BuiltinFunction& function) {
  switch (function) {
    case BuiltinFunction::Sin:
      return os << "Sin";
    case BuiltinFunction::Cos:
      return os << "Cos";
    case BuiltinFunction::Abs:
      return os << "Abs";
    case BuiltinFunction::Pow:
      return os << "Pow";
    case BuiltinFunction::Sqrt:
      return os << "Sqrt";
  }

  assert(false);
  return os;
}

std::ostream& operator<<(std::ostream& os, const Bytecode& bytecode) {
  os << "Bytecode(" << bytecode.kind() << ", ";
  switch (bytecode.kind()) {
    case BytecodeKind::LabelId:
      os << bytecode.labelId();
      break;
    case BytecodeKind::ArgumentCount:
      os << bytecode.argumentCount();
      break;
    case BytecodeKind::BuiltinFunctionId:
      os << bytecode.function();
      break;
    case BytecodeKind::Value:
      os << bytecode.value();
      break;
    case BytecodeKind::Instruction:
      os << bytecode.instruction();
      break;
    case BytecodeKind::Offset:
      os << bytecode.offset();
      break;
  }

  return os << ")";
}
