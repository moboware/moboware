#include "benchmark/benchmark.h"
#include "common/log_stream.h"
#include "modules/matching_engine_module/order_event_processor.h"

using namespace moboware::modules;

class IOrderHandlerMock : public IOrderHandler
{
public:
  void HandleOrderInsert(const OrderData& orderInsert, const boost::asio::ip::tcp::endpoint& endpoint) {}

  void HandleOrderAmend(const OrderAmendData& orderInsert, const boost::asio::ip::tcp::endpoint& endpoint){};

  void HandleOrderCancel(const OrderCancelData& orderCancel, const boost::asio::ip::tcp::endpoint& endpoint){};

  void GetOrderBook(const std::string&, const boost::asio::ip::tcp::endpoint&) {}
};

static void BM_OrderEventProcessorInsertOrder(benchmark::State& state)
{
  LogStream::GetInstance().SetLevel(LogStream::ERROR);

  const boost::asio::ip::tcp::endpoint endpoint;
  const auto orderHandlerMock{ std::make_shared<IOrderHandlerMock>() };
  OrderEventProcessor orderEventProcessor(orderHandlerMock, endpoint);

  boost::beast::flat_buffer buffer(10 * 1'024U);
  const std::string orderRequest{
    "{\"Action\":\"Insert\",\"Data\":{\"Account\":\"mobo\",\"Instrument\":"
    "\"ABCN\",\"Price\":100500000,\"Volume\":55,\"IsBuy\":false,\"Type\":\"Limit\",\"ClientId\":\"1298749274982713\"}}"
  };
  memcpy(buffer.prepare(orderRequest.size()).data(), orderRequest.c_str(), orderRequest.size());
  buffer.commit(orderRequest.size());
  for (const auto _ : state) {
    orderEventProcessor.Process(buffer);
  }
  buffer.clear();
}

BENCHMARK(BM_OrderEventProcessorInsertOrder);

static void BM_OrderEventProcessorCancelOrder(benchmark::State& state)
{
  LogStream::GetInstance().SetLevel(LogStream::ERROR);

  const boost::asio::ip::tcp::endpoint endpoint;
  const auto orderHandlerMock{ std::make_shared<IOrderHandlerMock>() };
  OrderEventProcessor orderEventProcessor(orderHandlerMock, endpoint);

  boost::beast::flat_buffer buffer(10 * 1'024U);
  const std::string orderRequest{
    "{\"Action\":\"Cancel\",\"Data\":{\"Instrument\":\"ABCN\",\"Price\":100500000,\"IsBuy\":false,\"Id\":\"309458290485\",\"ClientId\":\"1298749274982713\"}}"
  };
  memcpy(buffer.prepare(orderRequest.size()).data(), orderRequest.c_str(), orderRequest.size());
  buffer.commit(orderRequest.size());
  for (const auto _ : state) {
    orderEventProcessor.Process(buffer);
  }
  buffer.clear();
}

BENCHMARK(BM_OrderEventProcessorCancelOrder);

static void BM_OrderEventProcessorAmendOrder(benchmark::State& state)
{
  LogStream::GetInstance().SetLevel(LogStream::ERROR);

  const boost::asio::ip::tcp::endpoint endpoint;
  const auto orderHandlerMock{ std::make_shared<IOrderHandlerMock>() };
  OrderEventProcessor orderEventProcessor(orderHandlerMock, endpoint);

  boost::beast::flat_buffer buffer(10 * 1'024U);
  const std::string orderRequest{
    "{\"Action\":\"Amend\",\"Data\":{\"Account\":\"mobo\",\"Instrument\":"
    "\"ABCN\",\"Price\":100500000,\"NewPrice\":200500000,\"Volume\":55,\"NewVolume\":105,\"IsBuy\":false,\"Type\":\"Limit\",\"Id\":\"03495869043\","
    "\"ClientId\":\"1298749274982713\"}}"
  };
  memcpy(buffer.prepare(orderRequest.size()).data(), orderRequest.c_str(), orderRequest.size());
  buffer.commit(orderRequest.size());
  for (const auto _ : state) {
    orderEventProcessor.Process(buffer);
  }
  buffer.clear();
}

BENCHMARK(BM_OrderEventProcessorAmendOrder);

static void BM_OrderEventProcessorGetBook(benchmark::State& state)
{
  LogStream::GetInstance().SetLevel(LogStream::ERROR);

  const boost::asio::ip::tcp::endpoint endpoint;
  const auto orderHandlerMock{ std::make_shared<IOrderHandlerMock>() };
  OrderEventProcessor orderEventProcessor(orderHandlerMock, endpoint);

  boost::beast::flat_buffer buffer(10 * 1'024U);
  const std::string orderRequest{ "{\"Action\":\"GetBook\",\"Data\":{\"Instrument\":\"ABCN\"}}" };
  memcpy(buffer.prepare(orderRequest.size()).data(), orderRequest.c_str(), orderRequest.size());
  buffer.commit(orderRequest.size());
  for (const auto _ : state) {
    orderEventProcessor.Process(buffer);
  }
  buffer.clear();
}

BENCHMARK(BM_OrderEventProcessorGetBook);