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

#include <cassert>
#include <cstdint>
#include <ostream>

enum class ValueType : uint8_t {
  Integer,
  Float,
};

class Value {
 public:
  static Value createInt(int64_t integer) {
    Value ret(ValueType::Integer);
    ret.m_integer = integer;
    return ret;
  }

  static Value createDouble(double value) {
    Value ret(ValueType::Float);
    ret.m_double = value;
    return ret;
  }

  ValueType type() const { return m_type; }

  int64_t intValue() const {
    assert(type() == ValueType::Integer);
    return m_integer;
  }

  double doubleValue() const {
    assert(type() == ValueType::Float);
    return m_double;
  }

  double normalizedValue() const {
    switch (type()) {
      case ValueType::Float:
        return doubleValue();
      case ValueType::Integer:
        return intValue();
    }
    assert(false);
    return 0.0;
  }

 private:
  explicit Value(ValueType type) : m_type(type){};

  ValueType m_type;
  union {
    int64_t m_integer;
    double m_double;
  };
};

std::ostream& operator<<(std::ostream&, const Value&);
std::ostream& operator<<(std::ostream& os, const ValueType& type);
