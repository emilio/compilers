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
  /**
   * Stores a value in to a more permanent address, such as a named variable.
   *
   * Also leaves the value on the stack.
   *
   * Always followed by a LabelId.
   */
  StoreVar,
  /** Loads a variable into the top of the stack */
  LoadVar,
  /**
   * Clears a variable because it's gone out of scope.
   *
   * TODO(emilio): We can get rid of this if we don't have any kind of
   * destructor, _and_ if we store the variables using a free-list (to avoid
   * leaking).
   *
   * Always followed by a LabelId.
   */
  ClearVar,
  /** Add the two values at the top of the stack */
  Add,
  /** Subtract the two values at the top of the stack */
  Subtract,
  /** Multiply the two values at the top of the stack */
  Mul,
  /** Divide the two values at the top of the stack */
  Div,
  /** Do an unconditional jump, always followed by an `Offset`. */
  Jump,
  /**
   * Do an conditional jump, if the last value on the stack is zero.
   *
   * Always followed by an `Offset`.
   */
  JumpIfZero,
  /**
   * Call a builtin function.
   *
   * It's followed by an external function ID, the argument count, and as many
   * values as needed will be in the stack.
   */
  CallFunction,
};

std::ostream& operator<<(std::ostream&, const Instruction&);

typedef uint64_t LabelId;

/**
 * The kind of value that can appear in a program converted into bytecode.
 */
enum class BytecodeKind : uint8_t {
  /** A named label ID, for a variable, for example */
  LabelId,
  /** A builtin function */
  BuiltinFunctionId,
  /** The argument count for a function call. */
  ArgumentCount,
  /** A `value` */
  Value,
  /** A simple instruction */
  Instruction,
  /** An offset */
  Offset,
};

std::ostream& operator<<(std::ostream&, const BytecodeKind&);

enum class BuiltinFunction {
  Cos,
  Sin,
  Abs,
  Sqrt,
  Pow,
};

std::ostream& operator<<(std::ostream&, const BuiltinFunction&);

class Bytecode final {
  BytecodeKind m_kind;
  union Inner {
    Instruction m_instruction;
    BuiltinFunction m_builtinFunction;
    LabelId m_label;
    Value m_value;
    ssize_t m_offset;
    size_t m_argumentCount;

    Inner() {}
  } m_inner;

  explicit Bytecode(BytecodeKind kind) : m_kind(kind) {}

 public:
  explicit Bytecode(Value val) : m_kind(BytecodeKind::Value) {
    m_inner.m_value = std::move(val);
  }

  explicit Bytecode(Instruction ins) : m_kind(BytecodeKind::Instruction) {
    m_inner.m_instruction = ins;
  }

  static Bytecode offset(ssize_t offset) {
    Bytecode b(BytecodeKind::Offset);
    b.m_inner.m_offset = offset;
    return b;
  }

  static Bytecode argumentCount(size_t count) {
    Bytecode b(BytecodeKind::ArgumentCount);
    b.m_inner.m_argumentCount = count;
    return b;
  }

  static Bytecode label(LabelId id) {
    Bytecode b(BytecodeKind::LabelId);
    b.m_inner.m_label = id;
    return b;
  }

  static Bytecode function(BuiltinFunction fn) {
    Bytecode b(BytecodeKind::BuiltinFunctionId);
    b.m_inner.m_builtinFunction = fn;
    return b;
  }

  BytecodeKind kind() const { return m_kind; }

  Instruction instruction() const {
    assert(kind() == BytecodeKind::Instruction);
    return m_inner.m_instruction;
  }
  BuiltinFunction function() const {
    assert(kind() == BytecodeKind::BuiltinFunctionId);
    return m_inner.m_builtinFunction;
  }
  LabelId labelId() const {
    assert(kind() == BytecodeKind::LabelId);
    return m_inner.m_label;
  }
  const Value& value() const {
    assert(kind() == BytecodeKind::Value);
    return m_inner.m_value;
  }
  ssize_t offset() const {
    assert(kind() == BytecodeKind::Offset);
    return m_inner.m_offset;
  }
  size_t argumentCount() const {
    assert(kind() == BytecodeKind::ArgumentCount);
    return m_inner.m_argumentCount;
  }
};

std::ostream& operator<<(std::ostream&, const Bytecode&);
