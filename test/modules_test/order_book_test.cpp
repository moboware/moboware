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

class ChannelInterfaceMock : public moboware::common::ChannelInterface
{
public:
  MOCK_METHOD(void, SendWebSocketData, (const boost::asio::const_buffer& readBuffer, const boost::asio::ip::tcp::endpoint& endpoint));
};

class MatchingEngineMock : public MatchingEngine
{
public:
  explicit MatchingEngineMock(const std::shared_ptr<moboware::common::ChannelInterface>& channelInterface)
    : MatchingEngine(channelInterface)
  {
  }

  MOCK_METHOD(void, CreateAndSendMessage, (const moboware::modules::OrderInsertReply&, const boost::asio::ip::tcp::endpoint&));
  MOCK_METHOD(void, CreateAndSendMessage, (const moboware::modules::Trade&, const boost::asio::ip::tcp::endpoint& endpoint));
  MOCK_METHOD(void, CreateAndSendMessage, (const moboware::modules::ErrorReply&, const boost::asio::ip::tcp::endpoint& endpoint));
};

TEST_F(OrderBookTest, InsertBidOrdersTest)
{
  OrderBidBook_t orderBook;
  OrderData orderData;
  constexpr auto MAX_ORDER{ 10U };
  constexpr PriceType_t price{ 10U * std::mega::num };

  for (int i = { 0U }; i < MAX_ORDER; i++) {
    orderData.account = "mobo";
    orderData.IsBuySide = true;
    orderData.price = price;
    orderData.volume = 10;
    orderData.type = "Limit";
    orderData.orderTime = std::chrono::high_resolution_clock::now();

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
  constexpr PriceType_t price{ 10U * std::mega::num };

  for (int i = { 0U }; i < MAX_ORDER; i++) {
    orderData.account = "mobo";
    orderData.IsBuySide = false;
    orderData.price = price;
    orderData.volume = 10;
    orderData.type = "Limit";
    orderData.orderTime = std::chrono::high_resolution_clock::now();

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

TEST_F(OrderBookTest, MatchOrdersFullTradeAskSideTest)
{
  const auto channelInterface{ std::make_shared<ChannelInterfaceMock>() };
  MatchingEngineMock matchingEngine(channelInterface);

  const boost::asio::ip::tcp::endpoint endpoint;
  constexpr PriceType_t price{ 10U * std::mega::num };

  // expect 2 order insert replies and 2 trades
  // create bid order
  const OrderData orderDataBid{ "accountBid",
                                "ABCD",
                                price,
                                100,
                                "Limit",
                                true,
                                std::chrono::high_resolution_clock::now(),
                                std::chrono::milliseconds::duration::zero(),
                                "id=12938471298",
                                "clientId=2394857234" };

  // create ask order
  const OrderData orderDataAsk{ "accountAsk",
                                "ABCD",
                                price,
                                10,
                                "Limit",
                                false,
                                std::chrono::high_resolution_clock::now(),
                                std::chrono::milliseconds::duration::zero(),
                                "id=129384712934528",
                                "clientId=23948572456434" };

  const OrderInsertReply bidReply{ orderDataBid.id, orderDataBid.clientId };
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(bidReply, endpoint));

  const OrderInsertReply askReply{ orderDataAsk.id, orderDataAsk.clientId };
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(askReply, endpoint));
  // trade expectations
  const Trade tradeBid{ orderDataBid.account, orderDataBid.price, orderDataAsk.volume, orderDataBid.clientId, orderDataBid.id };
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(tradeBid, endpoint));

  const Trade tradeAsk{ orderDataAsk.account, orderDataAsk.price, orderDataAsk.volume, orderDataAsk.clientId, orderDataAsk.id };
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(tradeAsk, endpoint));

  // insert bid order
  ASSERT_TRUE(orderDataBid.Validate());
  matchingEngine.OrderInsert(orderDataBid, endpoint);

  // insert ask order
  ASSERT_TRUE(orderDataAsk.Validate());
  matchingEngine.OrderInsert(orderDataAsk, endpoint);

  const auto& bidOrderBook{ matchingEngine.GetBidOrderBook() };
  EXPECT_FALSE(bidOrderBook.GetOrderBookMap().empty());

  const auto& askOrderBook{ matchingEngine.GetAskOrderBook() };
  EXPECT_TRUE(askOrderBook.GetOrderBookMap().empty());
}

TEST_F(OrderBookTest, MatchOrdersFullTradeBidSideTest)
{
  const auto channelInterface{ std::make_shared<ChannelInterfaceMock>() };
  MatchingEngineMock matchingEngine(channelInterface);

  const boost::asio::ip::tcp::endpoint endpoint;
  constexpr PriceType_t price{ 10U * std::mega::num };

  const OrderData orderDataBid{ "mobo",
                                "ABCD",
                                price,
                                10,
                                "Limit",
                                true,
                                std::chrono::high_resolution_clock::now(),
                                std::chrono::milliseconds::duration::zero(),
                                "id=12938471298",
                                "clientId=2394857234" };

  const OrderData orderDataAsk{ "mobo",
                                "ABCD",
                                price,
                                100,
                                "Limit",
                                false,
                                std::chrono::high_resolution_clock::now(),
                                std::chrono::milliseconds::duration::zero(),
                                "id=13894751567856",
                                "clientId=jko5ynkl345326751389475189" };

  // expect 2 order insert replies and 2 trades
  const OrderInsertReply bidReply{ orderDataBid.id, orderDataBid.clientId };
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(bidReply, endpoint));

  const OrderInsertReply askReply{ orderDataAsk.id, orderDataAsk.clientId };
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(askReply, endpoint));

  // trade expectations
  const Trade tradeBid{ orderDataBid.account, orderDataBid.price, orderDataBid.volume, orderDataBid.clientId, orderDataBid.id };
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(tradeBid, endpoint));

  const Trade tradeAsk{ orderDataAsk.account, orderDataAsk.price, orderDataBid.volume, orderDataAsk.clientId, orderDataAsk.id };
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(tradeAsk, endpoint));

  // insert bid order
  ASSERT_TRUE(orderDataBid.Validate());
  matchingEngine.OrderInsert(orderDataBid, endpoint);

  // insert ask order
  ASSERT_TRUE(orderDataAsk.Validate());
  matchingEngine.OrderInsert(orderDataAsk, endpoint);

  const auto& bidOrderBook{ matchingEngine.GetBidOrderBook() };
  EXPECT_TRUE(bidOrderBook.GetOrderBookMap().empty());

  const auto& askOrderBook{ matchingEngine.GetAskOrderBook() };
  EXPECT_FALSE(askOrderBook.GetOrderBookMap().empty());
  EXPECT_EQ(askOrderBook.GetOrderBookMap().begin()->second.GetTopLevel().value().volume, 90);
}

TEST_F(OrderBookTest, MatchOrderFullTradeBidAndAskSideTest)
{
  const auto channelInterface{ std::make_shared<ChannelInterfaceMock>() };
  MatchingEngineMock matchingEngine(channelInterface);

  const boost::asio::ip::tcp::endpoint endpoint;
  constexpr PriceType_t price{ 10U * std::mega::num };

  const OrderData orderDataBid{ "mobo",
                                "ABCD",
                                price,
                                100,
                                "Limit",
                                true,
                                std::chrono::high_resolution_clock::now(),
                                std::chrono::milliseconds::duration::zero(),
                                "id=12938471298",
                                "clientId=2394857234" };

  const OrderData orderDataAsk{ "mobo",
                                "ABCD",
                                price,
                                100,
                                "Limit",
                                false,
                                std::chrono::high_resolution_clock::now(),
                                std::chrono::milliseconds::duration::zero(),
                                "id=13894751567856",
                                "clientId=jko5ynkl345326751389475189" };

  // expect 2 order insert replies and 2 trades
  const OrderInsertReply bidReply{ orderDataBid.id, orderDataBid.clientId };
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(bidReply, endpoint));

  const OrderInsertReply askReply{ orderDataAsk.id, orderDataAsk.clientId };
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(askReply, endpoint));

  // trade expectations
  const Trade tradeBid{ orderDataBid.account, orderDataBid.price, orderDataBid.volume, orderDataBid.clientId, orderDataBid.id };
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(tradeBid, endpoint));

  const Trade tradeAsk{ orderDataAsk.account, orderDataAsk.price, orderDataAsk.volume, orderDataAsk.clientId, orderDataAsk.id };
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(tradeAsk, endpoint));

  // insert bid side
  ASSERT_TRUE(orderDataBid.Validate());
  matchingEngine.OrderInsert(orderDataBid, endpoint);

  // insert ask side
  ASSERT_TRUE(orderDataAsk.Validate());
  matchingEngine.OrderInsert(orderDataAsk, endpoint);

  const auto& bidOrderBook{ matchingEngine.GetBidOrderBook() };
  EXPECT_TRUE(bidOrderBook.GetOrderBookMap().empty());

  const auto& askOrderBook{ matchingEngine.GetAskOrderBook() };
  EXPECT_TRUE(askOrderBook.GetOrderBookMap().empty());
}
