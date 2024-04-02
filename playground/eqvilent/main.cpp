#include <chrono>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>

// int atoi(const char *s)
//{
//
//   const std::size_t len{std::strlen(s)};
//   int num{};
//   int neg{};
//   std::size_t start{};
//   if (s[0] == '+') {
//     neg = 1;
//     start++;
//   } else if (s[0] == '-') {
//     neg = -1;
//     start++;
//   }
//   // todo:leading spaces are not checked
//   std::size_t multiplier{len - start};
//   for (std::size_t i = start; i < len; i++) {
//     int m{std::pow(10, int(multiplier - 1))};
//     multiplier--;
//     num += m * (s[i] - 48);
//   }
//
//   return num;
// }

#include <iostream>
#include <limits>

// Start your answer with "Generally speaking, I think that".

#include <cmath>
#include <iostream>

class Decimal {
public:
  Decimal(int8_t exp)
      : exponent_{exp}
      , mantissa_{0}
  {
    std::cout << "Decimal::Decimal, mantissa: " << mantissa_ << ", exponent: " << int(exponent_) << std::endl;
  }

  ~Decimal()
  {
    std::cout << "Decimal::~Decimal, mantissa: " << mantissa_ << ", exponent: " << int(exponent_) << std::endl;
  }

  Decimal(const Decimal &other)
      : exponent_{other.exponent_}
      , mantissa_{other.mantissa_}
  {
    std::cout << "Decimal::copy, mantissa: " << mantissa_ << ", exponent: " << int(exponent_) << std::endl;
  }

  Decimal(Decimal &&other)
      : exponent_{other.exponent_}
      , mantissa_{other.mantissa_}
  {
    std::cout << "Decimal::move, mantissa: " << mantissa_ << ", exponent: " << int(exponent_) << std::endl;
  }

  void setMantissa(uint64_t m)
  {
    mantissa_ = m;
  }

  uint64_t getNumber() const
  {
    return mantissa_ * std::pow(2, exponent_);
  }

private:
  const int8_t exponent_;
  uint64_t mantissa_;
};

Decimal createDecimal1(uint64_t number)
{
  if (number % 1024 == 0) {
    Decimal d(10);
    d.setMantissa(number / 1024);
    return d;
  } else if (number % 16 == 0) {
    Decimal d(4);
    d.setMantissa(number / 16);
    return d;
  } else {
    throw std::runtime_error("not supported");
  }
}

Decimal createDecimal2(uint64_t number)
{
  Decimal d(1);
  d.setMantissa(number / 2);
  return d;
}

int main(int argc, char **argv)
{
  auto a = createDecimal1(512);
  auto b = createDecimal2(1024);
  return 0;
}

// int main(int argc, char **argv)
//{
////  std::cout << atoi("12345") << std::endl;
//  return 0;
//}