#include <array>
#include <deque>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace internal {

template <typename TIter> int getDistance(const TIter &iter1, const TIter &iter2, std::random_access_iterator_tag)
{
  std::cout << "Random access" << std::endl;

  return iter2 - iter1;
}

template <typename TIter> int getDistance(const TIter &iter1, const TIter &iter2, std::forward_iterator_tag)
{
  std::cout << "Forward access" << std::endl;
  int dist{};
  for (auto i = iter1; i != iter2; ++i) {
    dist++;
  }
  return dist;
}

template <typename TIter, typename std::enable_if_t<std::is_base_of<TIter, std::forward_iterator_tag()>::value>>   //
int Distance(const TIter &iter1, const TIter &iter2)
{
  return 0;   //
}

}   // namespace internal

template <typename TIter> int Distance(const TIter &iter1, const TIter &iter2)
{
  return internal::getDistance(iter1, iter2, typename std::iterator_traits<TIter>::iterator_category());
}

template <typename TIter>   //
int _Distance(TIter iter1, TIter iter2)
{
  std::cout
      << typeid(typename std::iterator_traits<TIter>::iterator_category()).name() << ":" << std::boolalpha
      << std::is_base_of_v<typename std::iterator_traits<TIter>::iterator_category(), std::input_iterator_tag>   //
      << ","
      << std::is_same_v<typename std::iterator_traits<TIter>::iterator_category(), std::random_access_iterator_tag>   //
      << std::endl;

  // if constexpr (std::enable_if_t<
  //                   std::is_base_of_v<std::iterator_traits<TIter>::iterator_category(),
  //                   std::random_access_iterator_tag()>, bool> == true) {
  //   std::cout << "random _Distance" << std::endl;
  //   return 0;   // iter2 - iter1;
  // } else
  if constexpr (std::derived_from<TIter, std::forward_iterator_tag()> == true) {
    std::cout << "Forward access" << std::endl;
    int dist{};
    for (auto i = iter1; i != iter2; ++i) {
      dist++;
    }
    return dist;
  }
  return 0;
}

int main(int, char **)
{
  // vector
  using IntV_t = std::vector<int>;
  const IntV_t v1{0, 23, 4, 5, 7, 8, 89, 9, 90, 4, 3, 1};

  std::cout << "Vector Distance is:" << _Distance(v1.cbegin(), v1.cend()) << std::endl;
  // std::cout << "Vector Distance is:" << Distance(v1.cend(), v1.cbegin()) << std::endl;

  // deque
  using IntDeque_t = std::deque<int>;
  const IntDeque_t dq1{0, 23, 4, 5, 7, 8, 89, 9, 90, 4, 3, 1};

  std::cout << "Deque Distance is:" << Distance(dq1.cbegin(), dq1.cend()) << std::endl;
  // std::cout << "Deque Distance is:" << Distance(dq1.cend(), dq1.cbegin()) << std::endl;

  // array
  using IntA_t = std::array<int, 12>;
  const IntA_t a1{0, 23, 4, 5, 7, 8, 89, 9, 90, 4, 3, 1};

  std::cout << "Array Distance is:" << Distance(a1.cbegin(), a1.cend()) << std::endl;
  // std::cout << "Array Distance is:" << Distance(a1.cend(), a1.cbegin()) << std::endl;

  // map
  using IntMap_t = std::map<int, int>;
  const IntMap_t m1{
      {1, 2},
      {3, 4},
      {4, 5},
      {6, 7},
      {8, 9}
  };

  std::cout << "Map Distance is:" << Distance(m1.cbegin(), m1.cend()) << std::endl;

  // multi map
  using IntMMap_t = std::multimap<int, int>;
  const IntMMap_t mm1{
      {1, 2},
      {3, 4},
      {4, 5},
      {6, 7},
      {8, 9}
  };

  std::cout << "MultiMap Distance is:" << Distance(mm1.cbegin(), mm1.cend()) << std::endl;

  // unordered_map
  using IntUMap_t = std::unordered_map<int, int>;
  // IntUMap_t::iterator::iterator_category
  const IntUMap_t um1{
      {1, 2},
      {3, 4},
      {4, 5},
      {6, 7},
      {8, 9}
  };

  std::cout << "Unordered Map Distance is:" << Distance(um1.cbegin(), um1.cend()) << std::endl;

  // list
  using IntList_t = std::list<int>;
  const IntList_t l1{{1}, {3}, {4}, {6}, {8}, {12}, {2341}};

  std::cout << "List Distance is:" << Distance(++l1.cbegin(), l1.cend()) << std::endl;

  // set
  using IntSet_t = std::set<int>;
  const IntSet_t s1{{1}, {3}, {4}, {6}, {8}, {12}, {2341}, {3546}};

  std::cout << "Set Distance is:" << Distance(++s1.cbegin(), s1.cend()) << std::endl;

  return 0;
}