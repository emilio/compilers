#include "BytecodeCollector.h"
#include "Program.h"
#include "AST.h"

Result<std::unique_ptr<Program>, ProgramCreationError>
Program::fromAST(const ast::Node& ast) {
  BytecodeCollector collector;
  ast::BytecodeCollectionResult result = ast.toByteCode(collector);
  if (!result)
    return ProgramCreationError(result.unwrapErr());
  return std::unique_ptr<Program>(new Program(collector.takeBytecode()));
}

bool Program::execute(ExecutionContext& ctx) {
  return false;
}

std::ostream& operator<<(std::ostream& os, const Program& program) {
  os << "Program(\n";
  for (const auto& bytecode : program.m_bytecode)
    os << "  " << bytecode << '\n';
  return os << ")";
}
