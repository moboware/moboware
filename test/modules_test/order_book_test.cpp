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

TEST_F(OrderBookTest, MatchOrdersFullTradeAskSideTest)
{
  const auto channelInterface{ std::make_shared<ChannelInterfaceMock>() };
  MatchingEngine matchingEngine(channelInterface);

  // expect 2 order insert replies and 2 trades
  EXPECT_CALL(*channelInterface, SendWebSocketData(testing::_, testing::_)).Times(4);
  boost::asio::ip::tcp::endpoint endpoint;

  OrderData orderData;
  const PriceType_t price = 10 * 1e6;

  // insert bid side
  orderData.account = "mobo";
  orderData.IsBuySide = true;
  orderData.price = price;
  orderData.volume = 100;
  orderData.type = "Limit";
  orderData.orderTime = std::chrono::steady_clock::now();
  matchingEngine.OrderInsert(orderData, endpoint);

  // insert ask side
  orderData.IsBuySide = false;
  orderData.volume = 10;
  matchingEngine.OrderInsert(orderData, endpoint);

  const auto& bidOrderBook{ matchingEngine.GetBidOrderBook() };
  EXPECT_FALSE(bidOrderBook.GetOrderBookMap().empty());

  const auto& askOrderBook{ matchingEngine.GetAskOrderBook() };
  EXPECT_TRUE(askOrderBook.GetOrderBookMap().empty());
}

TEST_F(OrderBookTest, MatchOrdersFullTradeBidSideTest)
{
  const auto channelInterface{ std::make_shared<ChannelInterfaceMock>() };
  MatchingEngine matchingEngine(channelInterface);

  // expect 2 order insert replies and 2 trades
  EXPECT_CALL(*channelInterface, SendWebSocketData(testing::_, testing::_)).Times(4);
  boost::asio::ip::tcp::endpoint endpoint;

  OrderData orderData;
  const PriceType_t price = 10 * 1e6;

  // insert bid side
  orderData.account = "mobo";
  orderData.IsBuySide = true;
  orderData.price = price;
  orderData.volume = 10;
  orderData.type = "Limit";
  orderData.orderTime = std::chrono::steady_clock::now();
  matchingEngine.OrderInsert(orderData, endpoint);

  // insert ask side
  orderData.IsBuySide = false;
  orderData.volume = 100;
  matchingEngine.OrderInsert(orderData, endpoint);

  const auto& bidOrderBook{ matchingEngine.GetBidOrderBook() };
  EXPECT_TRUE(bidOrderBook.GetOrderBookMap().empty());

  const auto& askOrderBook{ matchingEngine.GetAskOrderBook() };
  EXPECT_FALSE(askOrderBook.GetOrderBookMap().empty());
  EXPECT_EQ(askOrderBook.GetOrderBookMap().begin()->second.GetTopLevel().value().volume, 90);
}

TEST_F(OrderBookTest, MatchOrderFullTradeBidAndAskSideTest)
{
  const auto channelInterface{ std::make_shared<ChannelInterfaceMock>() };
  MatchingEngine matchingEngine(channelInterface);

  // expect 2 order insert replies and 2 trades
  EXPECT_CALL(*channelInterface, SendWebSocketData(testing::_, testing::_)).Times(4);
  boost::asio::ip::tcp::endpoint endpoint;

  OrderData orderData;
  const PriceType_t price = 10 * 1e6;

  // insert bid side
  orderData.account = "mobo";
  orderData.IsBuySide = true;
  orderData.price = price;
  orderData.volume = 100;
  orderData.type = "Limit";
  orderData.orderTime = std::chrono::steady_clock::now();
  matchingEngine.OrderInsert(orderData, endpoint);

  // insert ask side
  orderData.IsBuySide = false;
  matchingEngine.OrderInsert(orderData, endpoint);

  const auto& bidOrderBook{ matchingEngine.GetBidOrderBook() };
  EXPECT_TRUE(bidOrderBook.GetOrderBookMap().empty());

  const auto& askOrderBook{ matchingEngine.GetAskOrderBook() };
  EXPECT_TRUE(askOrderBook.GetOrderBookMap().empty());
}
