#pragma once
#include "Tokenizer.h"

class TestReader final : public Reader {
 public:
  explicit TestReader(const char* str) : m_input(str), m_pos(0) {}

  char next() override {
    if (!m_input || !m_input[m_pos])
      return 0;
    return m_input[m_pos++];
  }

  ~TestReader() = default;

 private:
  const char* m_input;
  std::size_t m_pos;
};
