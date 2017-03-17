#pragma once

#include <cassert>
#include <cstdint>

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
  explicit Value(ValueType type) : m_type(type) {};

  ValueType m_type;
  union {
    int64_t m_integer;
    double m_double;
  };
};
