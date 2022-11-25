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

/**
 * @brief Test to check a multi level order book
 *     BID   | ASK
 * ------------------
 *            100@52 (2)
 *            100@51 (1)
 * (1) 100@50
 *
 * Bid Order insert 150@52, full match ask order 100@51 and ask partial match on ask order 100@52 for 50 volume
 * Will generate :
 * * 3 order insert replies
 * * 1 bid trade and 2 ask trades
 *
 * Post match order book state:
 *     BID   | ASK
 * ------------------
 *            50@52 (2)
 * (1) 100@50
 */
TEST_F(OrderBookTest, MultiLevelMatchOrderBidAndAskSideTest)
{
  const auto channelInterface{ std::make_shared<ChannelInterfaceMock>() };
  MatchingEngineMock matchingEngine(channelInterface);

  const boost::asio::ip::tcp::endpoint endpoint;
  const OrderData orderDataBid1{ "mobo",
                                 "ABCD",
                                 { 50U * std::mega::num },
                                 100,
                                 "Limit",
                                 true,
                                 std::chrono::high_resolution_clock::now(),
                                 std::chrono::milliseconds::duration::zero(),
                                 "id=12938471298",
                                 "clientId=2394857234" };
  ASSERT_TRUE(orderDataBid1.Validate());

  const OrderData orderDataAsk1{ "mobo",
                                 "ABCD",
                                 { 51U * std::mega::num },
                                 100,
                                 "Limit",
                                 false,
                                 std::chrono::high_resolution_clock::now(),
                                 std::chrono::milliseconds::duration::zero(),
                                 "id=13894751567856",
                                 "clientId=jko5ynkl345326751389475189" };
  ASSERT_TRUE(orderDataAsk1.Validate());

  const OrderData orderDataAsk2{ "mobo",
                                 "ABCD",
                                 { 52U * std::mega::num },
                                 100,
                                 "Limit",
                                 false,
                                 std::chrono::high_resolution_clock::now(),
                                 std::chrono::milliseconds::duration::zero(),
                                 "id=1389475156734958",
                                 "clientId=jko5ynkl34234532751389475189" };
  ASSERT_TRUE(orderDataAsk2.Validate());
  //
  const OrderInsertReply bidReply1{ orderDataBid1.id, orderDataBid1.clientId };
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(bidReply1, endpoint));

  const OrderInsertReply askReply1{ orderDataAsk1.id, orderDataAsk1.clientId };
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(askReply1, endpoint));

  const OrderInsertReply askReply2{ orderDataAsk2.id, orderDataAsk2.clientId };
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(askReply2, endpoint));

  matchingEngine.OrderInsert(orderDataBid1, endpoint);
  matchingEngine.OrderInsert(orderDataAsk1, endpoint);
  matchingEngine.OrderInsert(orderDataAsk2, endpoint);

  // insert bid order that will match multi level ask price
  const OrderData orderDataBid2{ "mobo",
                                 "ABCD",
                                 orderDataAsk2.price,
                                 150,
                                 "Limit",
                                 true,
                                 std::chrono::high_resolution_clock::now(),
                                 std::chrono::milliseconds::duration::zero(),
                                 "id=1293847129ertgetr8",
                                 "clientId=2394857yukityu234" };
  ASSERT_TRUE(orderDataBid2.Validate());

  const OrderInsertReply bidReply2{ orderDataBid2.id, orderDataBid2.clientId };
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(bidReply2, endpoint));

  const Trade tradeBid2{ orderDataBid2.account, orderDataBid2.price, orderDataAsk1.volume, orderDataBid2.clientId, orderDataBid2.id };
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(tradeBid2, endpoint));
  const Trade tradeAsk1{ orderDataAsk1.account, orderDataAsk1.price, orderDataAsk1.volume, orderDataAsk1.clientId, orderDataAsk1.id };
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(tradeAsk1, endpoint));

  matchingEngine.OrderInsert(orderDataBid2, endpoint);
  // check the state of the orderbook
  const auto& bidOrderBook{ matchingEngine.GetBidOrderBook() };
  EXPECT_EQ(bidOrderBook.GetOrderBookMap().size(), 1);
  EXPECT_EQ(bidOrderBook.GetOrderBookMap().begin()->second.GetTopLevel()->price, orderDataBid1.price);

  const auto& askOrderBook{ matchingEngine.GetAskOrderBook() };
  EXPECT_EQ(askOrderBook.GetOrderBookMap().size(), 1);
  EXPECT_EQ(askOrderBook.GetOrderBookMap().begin()->second.GetTopLevel()->price, orderDataAsk2.price);
}