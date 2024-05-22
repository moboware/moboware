#include "binance/binance_price_feed.hpp"

using namespace moboware;
using namespace moboware::exchanges::binance;

int main(int, char **)
{
  common::ServicePtr service{std::make_shared<common::Service>()};
  BinancePriceSessionHandler binancePriceSessionHandler;
  BinancePriceFeed binancePriceFeed(service, binancePriceSessionHandler);

  if (binancePriceFeed.Connect()) {
    service->Run();
  }
  return 0;
}