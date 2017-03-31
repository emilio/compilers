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

#include <cstdio>
#include "Tokenizer.h"

class FileReader final : public Reader {
  FILE* m_file;
  bool m_ownsHandle;

 public:
  FileReader(FILE* handle, bool owns) : m_file(handle), m_ownsHandle(owns) {}

  FileReader(const char* name) : FileReader(fopen(name, "r"), true) {}

  char next() override {
    if (!m_file || feof(m_file))
      return 0;
    char next = fgetc(m_file);
    if (next == EOF)
      return 0;
    return next;
  }

  ~FileReader() override {
    if (m_ownsHandle && m_file)
      fclose(m_file);
  }
};
