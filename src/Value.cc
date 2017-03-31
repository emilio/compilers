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

#include "Value.h"

std::ostream& operator<<(std::ostream& os, const ValueType& type) {
  switch (type) {
    case ValueType::Float:
      return os << "Float";
    case ValueType::Integer:
      return os << "Integer";
  }
  assert(false);
  return os;
}

std::ostream& operator<<(std::ostream& os, const Value& value) {
  return os << "Value(" << value.type() << ", " << value.normalizedValue()
            << ")";
}
