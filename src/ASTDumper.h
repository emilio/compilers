#pragma once

#include <ostream>

namespace ast {

class ASTDumper {
public:
  explicit ASTDumper(std::ostream& stream)
    : m_stream(stream)
    , m_indent(0)
  {}

  ASTDumper(ASTDumper& parent)
    : m_stream(parent.m_stream)
    , m_indent(parent.m_indent + 1)
  {
    if (!parent.m_hadChild)
      m_stream << "\n";
    parent.m_hadChild = true;
  }

  ~ASTDumper() {
    if (m_dirty && !m_hadChild)
      m_stream << "\n";
  }

private:
  std::ostream& m_stream;
  std::size_t m_indent;
  bool m_dirty { false };
  bool m_hadChild { false };

  template<typename T>
  friend ASTDumper& operator<<(ASTDumper& dumper, const T& v);
};

template<typename T>
ASTDumper& operator<<(ASTDumper& dumper, const T& v) {
  if (!dumper.m_dirty)
    for (std::size_t i = 0; i < dumper.m_indent; ++i)
      dumper.m_stream << " ";
  dumper.m_dirty = true;
  dumper.m_stream << v;
  return dumper;
}

}
