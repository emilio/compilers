#pragma once

#include "Executor.h"
#include "Program.h"

static Optional<Value> Executor::execute(const Program&,
                                         const ExecutionContext&) {
  return None;
}
