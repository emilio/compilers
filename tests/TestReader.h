/*
 * Copyright (C) 2017 Emilio Cobos Álvarez <emilio@crisal.io>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
