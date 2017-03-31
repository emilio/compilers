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
