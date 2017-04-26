#pragma once

#include <memory>
#include <vector>
#include "Bytecode.h"
#include "Result.h"

class Environment;
class ExecutionContext;

namespace ast {
  class Node;
}

class ProgramCreationError {
  std::string m_message;

 public:
  ProgramCreationError(std::string&& message)
    : m_message(std::move(message)) {}

  const std::string& message() const { return m_message; }
};

/**
 * A program is a compiled array of bytecode, compiled from a given AST node.
 */
class Program {
 public:
  /**
   * TODO(emilio): Need to figure out a nice interface for external functions.
   */
  static Result<std::unique_ptr<Program>, ProgramCreationError>
  fromAST(const ast::Node&);

  bool execute(ExecutionContext& ctx);

 private:
  Program(std::vector<Bytecode>&& bytecode)
    : m_bytecode(std::move(bytecode)) {}

  std::vector<Bytecode> m_bytecode;

  friend std::ostream& operator<<(std::ostream& os, const Program&);
};

std::ostream& operator<<(std::ostream& os, const Program&);
