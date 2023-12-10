#pragma once

#include <vector>

namespace moboware::common {

/// @brief Unordered fast map, is a map build from 2 vectors the are placed parallel, next to each other. One vector keeps
/// the keys and the second vector keeps the values. The keys and values are separated from each other in memory to keep the
/// cpu cache happy and the the level 1 cache can keep a continues key set in its cache.
/// Behaviour:
///   - All data is non sorted stored in the vectors
///   - The interface of the unordered fast map is mostly compatible with a std::map
///   - Range based for loop
///   - has an iterator that return first and second from the methods first() and second()
/// @tparam TKey
/// @tparam TValue

template <class TKey, class TValue> class unordered_fast_map {
public:
  using value_type = typename std::pair<const TKey, TValue>;
  using size_type = std::size_t;

  using keys_t = typename std::vector<TKey>;
  using key_iterator = typename std::vector<TKey>::iterator;
  using const_key_iterator = typename std::vector<TKey>::const_iterator;

  using value_t = typename std::vector<TValue>;
  using value_iterator = typename std::vector<TValue>::iterator;
  using const_value_iterator = typename std::vector<TValue>::const_iterator;

  class _iterator {
  public:
    _iterator(const_key_iterator keyIter, const_value_iterator valueIter)
        : m_CurrentKeyIter(keyIter)
        , m_CurrentValueIter(valueIter)
    {
    }

    // pre increment
    constexpr _iterator &operator++()
    {
      ++m_CurrentKeyIter;
      ++m_CurrentValueIter;

      return *this;
    }

    // post increment
    _iterator operator++(int) noexcept
    {
      return _iterator(++m_CurrentKeyIter, ++m_CurrentValueIter);
    }

    TKey &first()
    {
      return *m_CurrentKeyIter;
    }

    const TKey &first() const
    {
      return *m_CurrentKeyIter;
    }

    TValue &second()
    {
      return *m_CurrentValueIter;
    }

    const TValue &second() const
    {
      return *m_CurrentValueIter;
    }

    bool operator==(const _iterator &rhs) const
    {
      return rhs.m_CurrentKeyIter == this->m_CurrentKeyIter and   //
             rhs.m_CurrentValueIter == this->m_CurrentValueIter;
    }

    bool operator!=(const _iterator &rhs)
    {
      return !operator==(rhs);
    }

    std::pair<TKey, TValue> operator*() const noexcept
    {
      return {*m_CurrentKeyIter, *m_CurrentValueIter};
    }

  private:
    const_key_iterator m_CurrentKeyIter{};
    const_value_iterator m_CurrentValueIter{};
  };

  using iterator = _iterator;
  using const_iterator = const iterator;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  unordered_fast_map(const std::size_t size = 0)
  {
    m_Keys.reserve(size);
    m_Values.reserve(size);
  }

  constexpr std::pair<iterator, bool> insert(value_type &&p)
  {
    const auto iter1{m_Keys.insert(m_Keys.end(), p.first)};
    if (iter1 != m_Keys.end()) {
      const auto iter2{m_Values.insert(m_Values.end(), std::move(p.second))};
      return {
          {iter1, iter2},
          true
      };
    }
    return {
        {m_Keys.end(), m_Values.end()},
        false
    };
  }

  //  template <class... Args> std::pair<iterator, bool> try_emplace(TKey &&key, Args &&...values)
  //  {
  //    const auto iter{find(key)};
  //    if (end() == iter) {
  //      auto iter1{m_Keys.insert(m_Keys.end(), key)};
  //
  //      m_Values.emplace_back(values...);
  //      auto iter2{m_Values.end() - 1};
  //
  //      return {
  //          {iter1, iter2},
  //          true
  //      };
  //    }
  //    return {end(), false};
  //  }

  iterator find(const TKey &key)
  {
    for (auto &&iter1 = m_Keys.begin(), iter2 = m_Values.begin(); iter1 != m_Keys.end(); ++iter1, ++iter2) {
      if (*iter1 == key) {
        return iterator(iter1, iter2);
      }
    }
    return iterator(m_Keys.end(), m_Values.end());
  }

  const_iterator find(const TKey &key) const
  {
    return find(key);
    // for (auto &&iter1 = m_Keys.begin(), iter2 = m_Values.begin(); iter1 != m_Keys.end(); ++iter1, ++iter2) {
    //   if (*iter1 == key) {
    //     return const_iterator(iter1, iter2);
    //   }
    // }
    // return const_iterator(m_Keys.cend(), m_Values.cend());
  }

  iterator begin() noexcept
  {
    return iterator(m_Keys.begin(), m_Values.begin());
  }

  const_iterator begin() const noexcept
  {
    return const_iterator(m_Keys.cbegin(), m_Values.cbegin());
  }

  const_iterator cbegin() const noexcept
  {
    return std::make_pair(m_Keys.cbegin(), m_Values.cbegin());
  }

  iterator end() noexcept
  {
    return iterator(m_Keys.end(), m_Values.end());
  }

  const_iterator end() const noexcept
  {
    return const_iterator(m_Keys.cend(), m_Values.cend());
  }

  size_type size() const noexcept
  {
    return m_Keys.size();
  }

  [[nodiscard]] bool empty() const noexcept
  {
    return m_Keys.empty();
  }

  iterator erase(const TKey &key) noexcept
  {
    auto iter2 = m_Values.begin();

    for (auto &&iter1 = m_Keys.begin(); iter1 != m_Keys.end(); ++iter1, ++iter2) {
      if (*iter1 == key) {
        return iterator(m_Keys.erase(iter1), m_Values.erase(iter2));
      }
    }
    return end();
  }

  iterator erase(const iterator &iter)
  {
    return erase(iter.first());
  }

  void clear() noexcept
  {
    m_Keys.clear();
    m_Values.clear();
  }

private:
  keys_t m_Keys;
  value_t m_Values;
};
}   // namespace moboware::common