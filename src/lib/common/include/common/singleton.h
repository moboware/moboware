#pragma once

namespace moboware::common {
template <typename TClass> class Singleton {
public:
  static TClass &GetInstance()
  {
    static TClass instance;
    return instance;
  }

  Singleton() = default;
  ~Singleton() noexcept = default;
  Singleton(const Singleton &) = delete;
  Singleton &operator=(const Singleton &) = delete;
  Singleton(Singleton &&) = delete;
  Singleton &operator=(Singleton &&) = delete;
};
}   // namespace moboware::common