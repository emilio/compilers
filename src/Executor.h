#pragma once

#include "Optional.h"
#include "Value.h"

class Program;

class Executor final {
  static Optional<Value> execute(const Program&);
};
