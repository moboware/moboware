#include "benchmark/benchmark.h"
#include "common/log_stream.h"
#include "modules/matching_engine_module/matching_engine.h"
#include <random>

using namespace moboware;
using namespace moboware::modules;

class ChannelInterfaceMock : public common::ChannelInterface
{
public:
  void SendWebSocketData(const boost::asio::const_buffer& readBuffer, const boost::asio::ip::tcp::endpoint& endpoint) final{};
};

/**
 * @brief benchmark fixture
 */
class MatchingEngineBenchmark : public benchmark::Fixture
{
public:
  MatchingEngineBenchmark()
    : matchingEngine(std::make_shared<ChannelInterfaceMock>())
  {
  }

  void SetUp(const ::benchmark::State& state) { LogStream::GetInstance().SetLevel(LogStream::ERROR); }

  void TearDown(const ::benchmark::State& state) {}

  MatchingEngine matchingEngine;
  const boost::asio::ip::tcp::endpoint endpoint;
};

BENCHMARK_F(MatchingEngineBenchmark, InsertOrder)(benchmark::State& state)
{
  std::random_device rd;                                                 // Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd());                                                // Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<PriceType_t> priceDistribution(50, 100); // price range from 50..100

  for (const auto _ : state) {

    const OrderInsertData orderInsertData{ "mobo",
                                           "ABCD",
                                           { priceDistribution(gen) * std::mega::num },
                                           1'000U,
                                           "Limit",
                                           true,
                                           std::chrono::high_resolution_clock::now(),
                                           std::chrono::milliseconds::duration::zero(),
                                           std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count()),
                                           std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count()) };
    if (not orderInsertData.Validate()) {
      state.SkipWithError("Order data validation failed");
      return;
    }

    matchingEngine.OrderInsert(orderInsertData, endpoint);
  }
}

BENCHMARK_REGISTER_F(MatchingEngineBenchmark, InsertOrder)->DenseThreadRange(1, 8, 1);

BENCHMARK_F(MatchingEngineBenchmark, CancelOrder)(benchmark::State& state)
{

  for (const auto _ : state) {
    state.PauseTiming();

    std::random_device rd;                                                 // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd());                                                // Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<PriceType_t> priceDistribution(50, 100); // price range from 50..100

    const OrderInsertData orderInsertData{ "mobo",
                                           "ABCD",
                                           { priceDistribution(gen) * std::mega::num },
                                           1'000U,
                                           "Limit",
                                           true,
                                           std::chrono::high_resolution_clock::now(),
                                           std::chrono::milliseconds::duration::zero(),
                                           std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count()),
                                           std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count()) };
    if (not orderInsertData.Validate()) {
      state.SkipWithError("Order data validation failed");
      return;
    }

    matchingEngine.OrderInsert(orderInsertData, endpoint);

    OrderCancelData orderCancel{
      orderInsertData.GetInstrument(), orderInsertData.GetPrice(), orderInsertData.GetIsBuySide(), orderInsertData.GetId(), orderInsertData.GetClientId()
    };

    state.ResumeTiming();

    // start timing measurement here
    matchingEngine.OrderCancel(orderCancel, endpoint);
  }
}

BENCHMARK_REGISTER_F(MatchingEngineBenchmark, CancelOrder)->DenseThreadRange(1, 8, 1);
