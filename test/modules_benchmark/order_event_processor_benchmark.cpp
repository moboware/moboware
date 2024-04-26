#include "benchmark/benchmark.h"
#include "common/logger.hpp"
#include "modules/matching_engine_module/order_event_processor.h"

using namespace moboware::modules;

class IOrderHandlerMock : public IOrderHandler {
public:
  void HandleOrderInsert(OrderInsertData &&orderInsert, const boost::asio::ip::tcp::endpoint &endpoint) final
  {
  }

  void HandleOrderAmend(const OrderAmendData &orderInsert, const boost::asio::ip::tcp::endpoint &endpoint) final{};

  void HandleOrderCancel(const OrderCancelData &orderCancel, const boost::asio::ip::tcp::endpoint &endpoint) final{};

  void GetOrderBook(const std::string &, const boost::asio::ip::tcp::endpoint &) final
  {
  }
};

static auto orderHandlerMock{std::make_shared<IOrderHandlerMock>()};

class OrderEventProcessorBenchmark : public benchmark::Fixture {
public:
  OrderEventProcessorBenchmark()
      : orderEventProcessor(orderHandlerMock, boost::asio::ip::tcp::endpoint())
  {
  }

  void SetUp(const ::benchmark::State &state)
  {
    Logger::GetInstance().SetLevel(Logger::LogLevel::Error);
  }

  void TearDown(const ::benchmark::State &state)
  {
  }

  OrderEventProcessor orderEventProcessor;
  boost::beast::flat_buffer buffer{10 * 1'024U};
};

BENCHMARK_F(OrderEventProcessorBenchmark, InsertOrder)(benchmark::State &state)
{
  const std::string orderRequest{
      "{\"Action\":\"Insert\",\"Data\":{\"Account\":\"mobo\",\"Instrument\":"
      "\"ABCN\",\"Price\":100500000,\"Volume\":55,\"IsBuy\":false,\"Type\":\"Limit\",\"ClientId\":\"1298749274982713\"}}"};
  memcpy(buffer.prepare(orderRequest.size()).data(), orderRequest.c_str(), orderRequest.size());
  buffer.commit(orderRequest.size());
  for (const auto _ : state) {
    orderEventProcessor.Process(buffer);
  }
  buffer.clear();
}

BENCHMARK_REGISTER_F(OrderEventProcessorBenchmark, InsertOrder);   //->DenseThreadRange(1, 8, 1);

BENCHMARK_F(OrderEventProcessorBenchmark, CancelOrder)(benchmark::State &state)
{
  const std::string orderRequest{"{\"Action\":\"Cancel\",\"Data\":{\"Instrument\":\"ABCN\",\"Price\":100500000,\"IsBuy\":"
                                 "false,\"Id\":\"309458290485\",\"ClientId\":\"1298749274982713\"}}"};
  memcpy(buffer.prepare(orderRequest.size()).data(), orderRequest.c_str(), orderRequest.size());
  buffer.commit(orderRequest.size());
  for (const auto _ : state) {
    orderEventProcessor.Process(buffer);
  }
  buffer.clear();
}

BENCHMARK_REGISTER_F(OrderEventProcessorBenchmark, CancelOrder);   //->DenseThreadRange(1, 8, 1);

BENCHMARK_F(OrderEventProcessorBenchmark, AmendOrder)(benchmark::State &state)
{
  const std::string orderRequest{"{\"Action\":\"Amend\",\"Data\":{\"Account\":\"mobo\",\"Instrument\":"
                                 "\"ABCN\",\"Price\":100500000,\"NewPrice\":200500000,\"Volume\":55,\"NewVolume\":105,"
                                 "\"IsBuy\":false,\"Type\":\"Limit\",\"Id\":\"03495869043\","
                                 "\"ClientId\":\"1298749274982713\"}}"};
  memcpy(buffer.prepare(orderRequest.size()).data(), orderRequest.c_str(), orderRequest.size());
  buffer.commit(orderRequest.size());
  for (const auto _ : state) {
    orderEventProcessor.Process(buffer);
  }
  buffer.clear();
}

BENCHMARK_REGISTER_F(OrderEventProcessorBenchmark, AmendOrder);

BENCHMARK_F(OrderEventProcessorBenchmark, GetBook)(benchmark::State &state)
{

  const std::string orderRequest{"{\"Action\":\"GetBook\",\"Data\":{\"Instrument\":\"ABCN\"}}"};
  memcpy(buffer.prepare(orderRequest.size()).data(), orderRequest.c_str(), orderRequest.size());
  buffer.commit(orderRequest.size());
  for (const auto _ : state) {
    orderEventProcessor.Process(buffer);
  }
  buffer.clear();
}

BENCHMARK_REGISTER_F(OrderEventProcessorBenchmark, GetBook);   //->DenseThreadRange(1, 8, 1);
