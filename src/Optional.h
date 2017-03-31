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
#include <memory>

enum { None };

template <typename T>
class Optional {
  union {
    T m_value;
  };
  bool m_isSome;

 public:
  Optional() : Optional(None) {}

  Optional(decltype(None)) : m_isSome(false) {}

  Optional(Optional&& a_other) {
    m_isSome = a_other.isSome();
    if (m_isSome)
      m_value = std::move(a_other.m_value);
    a_other.m_isSome = false;
  }

  Optional& operator=(Optional&& a_other) {
    clear();
    m_isSome = a_other.isSome();
    if (m_isSome)
      m_value = std::move(a_other.value());
    a_other.m_isSome = false;
    return *this;
  }

  bool isSome() const { return m_isSome; }

  bool isNone() const { return !isSome(); }

  explicit operator bool() const { return isSome(); }

  T& value() {
    assert(isSome());
    return m_value;
  }

  const T& value() const { return const_cast<Optional*>(this)->value(); }

  T& operator*() { return value(); }

  const T& operator*() const { return value(); }

  T* operator->() { return &value(); }

  const T* operator->() const { return &value(); }

  template <typename... Args>
  void set(Args&&... aArgs) {
    m_isSome = true;
    new (&m_value) T(std::forward<Args>(aArgs)...);
  }

  void clear() {
    if (isSome()) {
      m_value.T::~T();
      m_isSome = false;
    }
  }

  ~Optional() { clear(); }
};

template <typename T>
Optional<T> Some(T value) {
  Optional<T> ret;
  ret.set(std::move(value));
  return std::move(ret);
}
