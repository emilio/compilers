#pragma once

#include <memory>

/**
 * A fairly dumb `Result` class, that allows you to either returns a value or an
 * error from a given value.
 */
template<typename OkType, typename ErrType>
class Result {
  bool m_isOk;

  union {
    OkType m_ok;
    ErrType m_err;
  };

 public:
  Result(OkType&& ok)
    : m_isOk(true)
    , m_ok(std::move(ok)) {}

  Result(ErrType&& err)
    : m_isOk(false)
    , m_err(std::move(err)) {}

  ~Result() {
    if (m_isOk)
      m_ok.~OkType();
    else
      m_err.~ErrType();
  }

  Result(Result&& other)
    : m_isOk(other.m_isOk) {
    if (m_isOk)
      m_ok = std::move(other.m_ok);
    else
      m_err = std::move(other.m_err);
  }

  bool isOk() const { return m_isOk; }
  bool isErr() const { return !isOk(); }

  OkType unwrap() {
    assert(isOk());
    return std::move(m_ok);
  }

  ErrType unwrapErr() {
    assert(isErr());
    return std::move(m_err);
  }

  explicit operator bool() {
    return isOk();
  }
};

/**
 * A generic Ok type.
 */
class Ok {};

#define TRY(expr) do { \
  auto __result = expr \
  if (!__result) \
    return Result(__result.unwrapErr()); \
} while (0)

#define TRY_VAR(target, expr) do { \
  auto __result = expr; \
  if (!__result) \
    return __result.unwrapErr(); \
  target = __result.unwrap(); \
} while (0)
