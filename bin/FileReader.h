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
