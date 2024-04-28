#include <chrono>
#include <iostream>

inline std::chrono::steady_clock::duration zeroDiff()
{
  const auto t1{std::chrono::steady_clock::now()};
  const auto t2{std::chrono::steady_clock::now()};

  std::cout << "SteadyClock now1  :" << t1.time_since_epoch() << ", SteadyClock now2:" << t2.time_since_epoch() << std::endl;
  ;

  return (t2 - t1);
}

inline std::chrono::steady_clock::duration systemNowDiffTime()
{
  const auto t1{std::chrono::steady_clock::now()};

  const auto now{std::chrono::system_clock::now()};

  const auto t2{std::chrono::steady_clock::now()};

  std::cout << "System now:" << now.time_since_epoch() << ":";

  return (t2 - t1);
}

int main(int, char **)
{
  std::cout << "zeroDiff:" << zeroDiff() << std::endl;
  std::cout << "systemNowDiff:" << systemNowDiffTime() << std::endl;

  return 0;
}