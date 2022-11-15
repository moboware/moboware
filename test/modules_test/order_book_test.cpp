#include "common/log_stream.h"
#include "modules/matching_engine_module/matching_engine.h"
#include "modules/matching_engine_module/order_book.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace moboware::modules;

class OrderBookTest : public testing::Test
{
public:
};

TEST_F(OrderBookTest, InsertBidOrdersTest)
{
  OrderBidBook_t orderBook;
  OrderData orderData;
  const auto MAX_ORDER{ 10U };
  const PriceType_t price = 10 * 1e6;

  for (int i = { 0U }; i < MAX_ORDER; i++) {
    orderData.account = "mobo";
    orderData.IsBuySide = true;
    orderData.price = price;
    orderData.volume = 10;
    orderData.type = "Limit";
    orderData.orderTime = std::chrono::steady_clock::now();

    std::stringstream strm;
    strm << i;
    orderData.id = strm.str();

    orderBook.Insert(orderData);
  }

  const auto levelOptional{ orderBook.GetLevelAtPrice(price) };
  EXPECT_TRUE(levelOptional.has_value());
  const auto* orderLevel = *levelOptional;
  EXPECT_EQ(orderLevel->GetSize(), MAX_ORDER);
}

TEST_F(OrderBookTest, InsertAskOrdersTest)
{
  OrderAskBook_t orderBook;
  OrderData orderData;
  const auto MAX_ORDER{ 10U };
  const PriceType_t price = 10 * 1e6;

  for (int i = { 0U }; i < MAX_ORDER; i++) {
    orderData.account = "mobo";
    orderData.IsBuySide = false;
    orderData.price = price;
    orderData.volume = 10;
    orderData.type = "Limit";
    orderData.orderTime = std::chrono::steady_clock::now();

    std::stringstream strm;
    strm << i;
    orderData.id = strm.str();

    orderBook.Insert(orderData);
  }

  const auto levelOptional{ orderBook.GetLevelAtPrice(price) };
  EXPECT_TRUE(levelOptional.has_value());
  const auto* orderLevel = *levelOptional;
  EXPECT_EQ(orderLevel->GetSize(), MAX_ORDER);
}

class ChannelInterfaceMock : public moboware::common::ChannelInterface
{
public:
  MOCK_METHOD(void, SendWebSocketData, (const boost::asio::const_buffer& readBuffer, const boost::asio::ip::tcp::endpoint& endpoint));
};

TEST_F(OrderBookTest, MatchOrdersTest)
{
  const auto channelInterface{ std::make_shared<ChannelInterfaceMock>() };
  MatchingEngine matchingEngine(channelInterface);

  OrderData orderData;
  const PriceType_t price = 10 * 1e6;

  orderData.account = "mobo";
  orderData.IsBuySide = true;
  orderData.price = price;
  orderData.volume = 100;
  orderData.type = "Limit";
  orderData.orderTime = std::chrono::steady_clock::now();

  EXPECT_CALL(*channelInterface, SendWebSocketData(testing::_, testing::_)).Times(2);
  boost::asio::ip::tcp::endpoint endpoint;
  matchingEngine.OrderInsert(orderData, endpoint);
  // ask side
  orderData.IsBuySide = false;
  orderData.volume = 10;

  matchingEngine.OrderInsert(orderData, endpoint);
}