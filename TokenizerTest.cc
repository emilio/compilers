#include <cstdio>
#include <iostream>
#include "Tokenizer.h"

// TODO(emilio): This should probably become a proper unit test with gtest or
// something like that.

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

int main(int, const char**) {
  FileReader reader(stdin, false);
  Tokenizer tokenizer(reader);

  while (true) {
    Token token = tokenizer.nextToken();
    std::cout << token << std::endl;
    if (token.type() == TokenType::Eof)
      break;
  }
}
