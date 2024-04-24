#include "common/log_stream.h"
#include "modules/matching_engine_module/order_event_processor.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace moboware::modules;

class IOrderHandlerMock : public IOrderHandler {
public:
  MOCK_METHOD(void, HandleOrderInsert, (OrderInsertData && orderInsert, const boost::asio::ip::tcp::endpoint &endpoint));

  MOCK_METHOD(void, HandleOrderAmend, (const OrderAmendData &orderInsert, const boost::asio::ip::tcp::endpoint &endpoint));

  MOCK_METHOD(void, HandleOrderCancel, (const OrderCancelData &orderCancel, const boost::asio::ip::tcp::endpoint &endpoint));

  MOCK_METHOD(void, GetOrderBook, (const std::string &instrument, const boost::asio::ip::tcp::endpoint &endpoint));
};

TEST(OrderEventProcessorTest, InsertOrderTest)
{
  const auto mock{std::make_shared<IOrderHandlerMock>()};
  OrderEventProcessor eventProcessor(mock, boost::asio::ip::tcp::endpoint());

  boost::beast::flat_buffer buffer{1 * 1'024U};
  const std::string orderRequest{
      "{\"Action\":\"Insert\",\"Data\":{\"Account\":\"mobo\",\"Instrument\":"
      "\"ABCN\",\"Price\":100500000,\"Volume\":55,\"IsBuy\":false,\"Type\":\"Limit\",\"ClientId\":\"1298749274982713\"}}"};

  memcpy(buffer.prepare(orderRequest.size()).data(), orderRequest.c_str(), orderRequest.size());
  buffer.commit(orderRequest.size());

  EXPECT_CALL(*mock, HandleOrderInsert(::testing::_, ::testing::_));
  eventProcessor.Process(buffer);
}

TEST(OrderEventProcessorTest, CancelOrderTest)
{
  const auto mock{std::make_shared<IOrderHandlerMock>()};
  OrderEventProcessor eventProcessor(mock, boost::asio::ip::tcp::endpoint());

  boost::beast::flat_buffer buffer{1 * 1'024U};
  const std::string orderRequest{"{\"Action\":\"Cancel\",\"Data\":{\"Instrument\":\"ABCN\",\"Price\":100500000,\"IsBuy\":"
                                 "false,\"Id\":\"309458290485\",\"ClientId\":\"1298749274982713\"}}"};

  memcpy(buffer.prepare(orderRequest.size()).data(), orderRequest.c_str(), orderRequest.size());
  buffer.commit(orderRequest.size());

  EXPECT_CALL(*mock, HandleOrderCancel(::testing::_, ::testing::_));
  eventProcessor.Process(buffer);
}

TEST(OrderEventProcessorTest, AmendOrderTest)
{
  const auto mock{std::make_shared<IOrderHandlerMock>()};
  OrderEventProcessor eventProcessor(mock, boost::asio::ip::tcp::endpoint());

  boost::beast::flat_buffer buffer{1 * 1'024U};
  const std::string orderRequest{"{\"Action\":\"Amend\",\"Data\":{\"Account\":\"mobo\",\"Instrument\":"
                                 "\"ABCN\",\"Price\":100500000,\"NewPrice\":200500000,\"Volume\":55,\"NewVolume\":105,"
                                 "\"IsBuy\":false,\"Type\":\"Limit\",\"Id\":\"03495869043\","
                                 "\"ClientId\":\"1298749274982713\"}}"};

  memcpy(buffer.prepare(orderRequest.size()).data(), orderRequest.c_str(), orderRequest.size());
  buffer.commit(orderRequest.size());

  EXPECT_CALL(*mock, HandleOrderAmend(::testing::_, ::testing::_));
  eventProcessor.Process(buffer);
}

TEST(OrderEventProcessorTest, GetOrderBookTest)
{
  const auto mock{std::make_shared<IOrderHandlerMock>()};
  OrderEventProcessor eventProcessor(mock, boost::asio::ip::tcp::endpoint());

  boost::beast::flat_buffer buffer{1 * 1'024U};
  const std::string orderRequest{"{\"Action\":\"GetBook\",\"Data\":{\"Instrument\":\"ABCN\"}}"};

  memcpy(buffer.prepare(orderRequest.size()).data(), orderRequest.c_str(), orderRequest.size());
  buffer.commit(orderRequest.size());

  EXPECT_CALL(*mock, GetOrderBook(::testing::_, ::testing::_));
  eventProcessor.Process(buffer);
}