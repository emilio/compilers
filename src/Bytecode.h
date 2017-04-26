#pragma once

#include <ostream>
#include "Value.h"

/**
 * Program execution consists of two parts:
 *
 *  * A execution stack, full of values.
 *  * Bytecode, a set of instructions that operate on the stack.
 *
 * After parsing, the AST is lowered into bytecode, and then executed.
 */
enum class Instruction : uint8_t {
  /**
   * Load a single value in the top of the stack.
   *
   * The instruction is followed by a `Value`.
   */
  Load,
  /**
   * Pop a single value from the top of the stack. The stack must be non-empty
   * when it happens.
   */
  Pop,
  /** Stores a value in to a more permanent address, such as a named variable */
  StoreVar,
  /** Loads a variable into the top of the stack */
  LoadVar,
  /** Add the two values at the top of the stack */
  Add,
  /** Subtract the two values at the top of the stack */
  Subtract,
  /** Multiply the two values at the top of the stack */
  Mul,
  /** Divide the two values at the top of the stack */
  Div,
  /**
   * Call an external function.
   *
   * It's followed by an external function ID, the argument count, and as many
   * values as needed.
   */
  CallExternalFunction,
};

std::ostream& operator<<(std::ostream&, const Instruction&);

typedef uint64_t LabelId;
typedef uint64_t ExternalFunctionId;

/**
 * The kind of value that can appear in a program converted into bytecode.
 */
enum class BytecodeKind : uint8_t {
  /** A named label ID */
  LabelId,
  /** An external function ID */
  ExternalFunctionId,
  /** A `value` */
  Value,
  /** A simple instruction */
  Instruction,
};

std::ostream& operator<<(std::ostream&, const BytecodeKind&);

class Bytecode final {
  BytecodeKind m_kind;
  union Inner {
    Instruction m_instruction;
    ExternalFunctionId m_functionId;
    LabelId m_label;
    Value m_value;

    Inner() {}
  } m_inner;

 public:
  explicit Bytecode(Value val) : m_kind(BytecodeKind::Value) {
    m_inner.m_value = std::move(val);
  }

  explicit Bytecode(Instruction ins) : m_kind(BytecodeKind::Instruction) {
    m_inner.m_instruction = ins;
  }

  BytecodeKind kind() const { return m_kind; }

  Instruction instruction() const {
    assert(kind() == BytecodeKind::Instruction);
    return m_inner.m_instruction;
  }
  ExternalFunctionId functionId() const {
    assert(kind() == BytecodeKind::ExternalFunctionId);
    return m_inner.m_functionId;
  }
  LabelId labelId() const {
    assert(kind() == BytecodeKind::LabelId);
    return m_inner.m_label;
  }
  const Value& value() const {
    assert(kind() == BytecodeKind::Value);
    return m_inner.m_value;
  }
};

std::ostream& operator<<(std::ostream&, const Bytecode&);
