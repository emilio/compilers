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

#include <ostream>

namespace ast {

class ASTDumper {
 public:
  explicit ASTDumper(std::ostream& stream) : m_stream(stream), m_indent(0) {}

  ASTDumper(ASTDumper& parent)
      : m_stream(parent.m_stream), m_indent(parent.m_indent + 1) {
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
  bool m_dirty{false};
  bool m_hadChild{false};

  template <typename T>
  friend ASTDumper& operator<<(ASTDumper& dumper, const T& v);
};

template <typename T>
ASTDumper& operator<<(ASTDumper& dumper, const T& v) {
  if (!dumper.m_dirty)
    for (std::size_t i = 0; i < dumper.m_indent; ++i)
      dumper.m_stream << " ";
  dumper.m_dirty = true;
  dumper.m_stream << v;
  return dumper;
}

}  // namespace ast
