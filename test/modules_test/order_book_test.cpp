#include "common/logger.hpp"
#include "modules/matching_engine_module/matching_engine.h"
#include "modules/matching_engine_module/order_book.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace moboware::modules;

class OrderBookTest : public testing::Test {
public:
};

class ChannelInterfaceMock : public moboware::common::ChannelInterface {
public:
  MOCK_METHOD(void,
              SendWebSocketData,
              (const boost::asio::const_buffer &readBuffer, const boost::asio::ip::tcp::endpoint &endpoint));
};

class MatchingEngineMock : public MatchingEngine {
public:
  explicit MatchingEngineMock(const std::shared_ptr<moboware::common::ChannelInterface> &channelInterface)
      : MatchingEngine(channelInterface)
  {
  }

  MOCK_METHOD(void, CreateAndSendMessage, (const moboware::modules::OrderReply &, const boost::asio::ip::tcp::endpoint &));
  MOCK_METHOD(void,
              CreateAndSendMessage,
              (const moboware::modules::Trade &, const boost::asio::ip::tcp::endpoint &endpoint));
  MOCK_METHOD(void,
              CreateAndSendMessage,
              (const moboware::modules::ErrorReply &, const boost::asio::ip::tcp::endpoint &endpoint));
};

TEST_F(OrderBookTest, InsertBidOrdersTest)
{
  OrderBidBook_t orderBook;

  constexpr auto MAX_ORDER{10U};
  constexpr PriceType_t price{10U * std::mega::num};

  for (int i = {0U}; i < MAX_ORDER; i++) {
    OrderInsertData orderData;
    orderData.SetAccount("mobo");
    orderData.SetIsBuySide(true);
    orderData.SetPrice(price);
    orderData.SetVolume(10);
    orderData.SetType("Limit");
    orderData.SetOrderTime(std::chrono::high_resolution_clock::now());

    std::stringstream strm;
    strm << i;
    orderData.SetId(strm.str());

    orderBook.Insert(std::move(orderData));
  }

  const auto levelOptional{orderBook.GetLevelAtPrice(price)};
  EXPECT_TRUE(levelOptional.has_value());
  const auto *orderLevel = *levelOptional;
  EXPECT_EQ(orderLevel->GetSize(), MAX_ORDER);
}

TEST_F(OrderBookTest, InsertAskOrdersTest)
{
  OrderAskBook_t orderBook;

  const auto MAX_ORDER{10U};
  constexpr PriceType_t price{10U * std::mega::num};

  for (int i = {0U}; i < MAX_ORDER; i++) {
    OrderInsertData orderData;
    orderData.SetAccount("mobo");
    orderData.SetIsBuySide(false);
    orderData.SetPrice(price);
    orderData.SetVolume(10);
    orderData.SetType("Limit");
    orderData.SetOrderTime(std::chrono::high_resolution_clock::now());

    std::stringstream strm;
    strm << i;
    orderData.SetId(strm.str());

    orderBook.Insert(std::move(orderData));
  }

  const auto levelOptional{orderBook.GetLevelAtPrice(price)};
  EXPECT_TRUE(levelOptional.has_value());
  const auto *orderLevel = *levelOptional;
  EXPECT_EQ(orderLevel->GetSize(), MAX_ORDER);
}

TEST_F(OrderBookTest, MatchOrdersFullTradeAskSideTest)
{
  const auto channelInterface{std::make_shared<ChannelInterfaceMock>()};
  MatchingEngineMock matchingEngine(channelInterface);

  const boost::asio::ip::tcp::endpoint endpoint;
  constexpr PriceType_t price{10U * std::mega::num};

  // expect 2 order insert replies and 2 trades
  // create bid order
  OrderInsertData orderDataBid{"accountBid",
                               "ABCD",
                               price,
                               100,
                               "Limit",
                               true,
                               std::chrono::high_resolution_clock::now(),
                               std::chrono::milliseconds::duration::zero(),
                               "id=12938471298",
                               "clientId=2394857234"};

  // create ask order
  OrderInsertData orderDataAsk{"accountAsk",
                               "ABCD",
                               price,
                               10,
                               "Limit",
                               false,
                               std::chrono::high_resolution_clock::now(),
                               std::chrono::milliseconds::duration::zero(),
                               "id=129384712934528",
                               "clientId=23948572456434"};

  const OrderReply bidReply{orderDataBid.GetId(), orderDataBid.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(bidReply, endpoint));

  const OrderReply askReply{orderDataAsk.GetId(), orderDataAsk.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(askReply, endpoint));
  // trade expectations
  Trade tradeBid{orderDataBid.GetAccount(),
                 orderDataBid.GetPrice(),
                 orderDataAsk.GetVolume(),
                 orderDataBid.GetId(),
                 orderDataBid.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(tradeBid, endpoint));

  Trade tradeAsk{orderDataAsk.GetAccount(),
                 orderDataAsk.GetPrice(),
                 orderDataAsk.GetVolume(),
                 orderDataAsk.GetId(),
                 orderDataAsk.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(tradeAsk, endpoint));

  // insert bid order
  ASSERT_TRUE(orderDataBid.Validate());
  matchingEngine.OrderInsert(std::move(orderDataBid), endpoint);

  // insert ask order
  ASSERT_TRUE(orderDataAsk.Validate());
  matchingEngine.OrderInsert(std::move(orderDataAsk), endpoint);

  const auto &bidOrderBook{matchingEngine.GetBidOrderBook()};
  EXPECT_FALSE(bidOrderBook.GetOrderBookMap().empty());

  const auto &askOrderBook{matchingEngine.GetAskOrderBook()};
  EXPECT_TRUE(askOrderBook.GetOrderBookMap().empty());
}

TEST_F(OrderBookTest, MatchOrdersFullTradeBidSideTest)
{
  const auto channelInterface{std::make_shared<ChannelInterfaceMock>()};
  MatchingEngineMock matchingEngine(channelInterface);

  const boost::asio::ip::tcp::endpoint endpoint;
  constexpr PriceType_t price{10U * std::mega::num};

  OrderInsertData orderDataBid{"mobo",
                               "ABCD",
                               price,
                               10,
                               "Limit",
                               true,
                               std::chrono::high_resolution_clock::now(),
                               std::chrono::milliseconds::duration::zero(),
                               "id=12938471298",
                               "clientId=2394857234"};

  OrderInsertData orderDataAsk{"mobo",
                               "ABCD",
                               price,
                               100,
                               "Limit",
                               false,
                               std::chrono::high_resolution_clock::now(),
                               std::chrono::milliseconds::duration::zero(),
                               "id=13894751567856",
                               "clientId=jko5ynkl345326751389475189"};

  // expect 2 order insert replies and 2 trades
  const OrderReply bidReply{orderDataBid.GetId(), orderDataBid.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(bidReply, endpoint));

  const OrderReply askReply{orderDataAsk.GetId(), orderDataAsk.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(askReply, endpoint));

  // trade expectations
  const Trade tradeBid{orderDataBid.GetAccount(),
                       orderDataBid.GetPrice(),
                       orderDataBid.GetVolume(),
                       orderDataBid.GetId(),
                       orderDataBid.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(tradeBid, endpoint));

  const Trade tradeAsk{orderDataAsk.GetAccount(),
                       orderDataAsk.GetPrice(),
                       orderDataBid.GetVolume(),
                       orderDataAsk.GetId(),
                       orderDataAsk.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(tradeAsk, endpoint));

  // insert bid order
  ASSERT_TRUE(orderDataBid.Validate());
  matchingEngine.OrderInsert(std::move(orderDataBid), endpoint);

  // insert ask order
  ASSERT_TRUE(orderDataAsk.Validate());
  matchingEngine.OrderInsert(std::move(orderDataAsk), endpoint);

  const auto &bidOrderBook{matchingEngine.GetBidOrderBook()};
  EXPECT_TRUE(bidOrderBook.GetOrderBookMap().empty());

  const auto &askOrderBook{matchingEngine.GetAskOrderBook()};
  EXPECT_FALSE(askOrderBook.GetOrderBookMap().empty());
  EXPECT_EQ(askOrderBook.GetOrderBookMap().begin()->second.GetTopLevel().value().GetVolume(), 90);
}

TEST_F(OrderBookTest, MatchOrderFullTradeBidAndAskSideTest)
{
  const auto channelInterface{std::make_shared<ChannelInterfaceMock>()};
  MatchingEngineMock matchingEngine(channelInterface);

  const boost::asio::ip::tcp::endpoint endpoint;
  constexpr PriceType_t price{10U * std::mega::num};

  OrderInsertData orderDataBid{"mobo",
                               "ABCD",
                               price,
                               100,
                               "Limit",
                               true,
                               std::chrono::high_resolution_clock::now(),
                               std::chrono::milliseconds::duration::zero(),
                               "id=12938471298",
                               "clientId=2394857234"};

  OrderInsertData orderDataAsk{"mobo",
                               "ABCD",
                               price,
                               100,
                               "Limit",
                               false,
                               std::chrono::high_resolution_clock::now(),
                               std::chrono::milliseconds::duration::zero(),
                               "id=13894751567856",
                               "clientId=jko5ynkl345326751389475189"};

  // expect 2 order insert replies and 2 trades
  const OrderReply bidReply{orderDataBid.GetId(), orderDataBid.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(bidReply, endpoint));

  const OrderReply askReply{orderDataAsk.GetId(), orderDataAsk.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(askReply, endpoint));

  // trade expectations
  const Trade tradeBid{orderDataBid.GetAccount(),
                       orderDataBid.GetPrice(),
                       orderDataBid.GetVolume(),
                       orderDataBid.GetId(),
                       orderDataBid.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(tradeBid, endpoint));

  const Trade tradeAsk{orderDataAsk.GetAccount(),
                       orderDataAsk.GetPrice(),
                       orderDataAsk.GetVolume(),
                       orderDataAsk.GetId(),
                       orderDataAsk.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(tradeAsk, endpoint));

  // insert bid side
  ASSERT_TRUE(orderDataBid.Validate());
  matchingEngine.OrderInsert(std::move(orderDataBid), endpoint);

  // insert ask side
  ASSERT_TRUE(orderDataAsk.Validate());
  matchingEngine.OrderInsert(std::move(orderDataAsk), endpoint);

  const auto &bidOrderBook{matchingEngine.GetBidOrderBook()};
  EXPECT_TRUE(bidOrderBook.GetOrderBookMap().empty());

  const auto &askOrderBook{matchingEngine.GetAskOrderBook()};
  EXPECT_TRUE(askOrderBook.GetOrderBookMap().empty());
}

TEST_F(OrderBookTest, MatchOrderFullTradeAskAndBidSideTest)
{
  // ask side fully matches the bid side
  const auto channelInterface{std::make_shared<ChannelInterfaceMock>()};
  MatchingEngineMock matchingEngine(channelInterface);

  const boost::asio::ip::tcp::endpoint endpoint;
  constexpr PriceType_t price{10U * std::mega::num};

  OrderInsertData orderDataBid{"mobo",
                               "ABCD",
                               price,
                               100,
                               "Limit",
                               true,
                               std::chrono::high_resolution_clock::now(),
                               std::chrono::milliseconds::duration::zero(),
                               "id=12938471298",
                               "clientId=2394857234"};

  OrderInsertData orderDataAsk{"mobo",
                               "ABCD",
                               price,
                               100,
                               "Limit",
                               false,
                               std::chrono::high_resolution_clock::now(),
                               std::chrono::milliseconds::duration::zero(),
                               "id=13894751567856",
                               "clientId=jko5ynkl345326751389475189"};

  // expect 2 order insert replies and 2 trades
  const OrderReply bidReply{orderDataBid.GetId(), orderDataBid.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(bidReply, endpoint));

  const OrderReply askReply{orderDataAsk.GetId(), orderDataAsk.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(askReply, endpoint));

  // trade expectations
  const Trade tradeBid{orderDataBid.GetAccount(),
                       orderDataBid.GetPrice(),
                       orderDataBid.GetVolume(),
                       orderDataBid.GetId(),
                       orderDataBid.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(tradeBid, endpoint));

  const Trade tradeAsk{orderDataAsk.GetAccount(),
                       orderDataAsk.GetPrice(),
                       orderDataAsk.GetVolume(),
                       orderDataAsk.GetId(),
                       orderDataAsk.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(tradeAsk, endpoint));

  // insert ask side first
  ASSERT_TRUE(orderDataAsk.Validate());
  matchingEngine.OrderInsert(std::move(orderDataAsk), endpoint);

  // insert bid side secondly
  ASSERT_TRUE(orderDataBid.Validate());
  matchingEngine.OrderInsert(std::move(orderDataBid), endpoint);

  const auto &bidOrderBook{matchingEngine.GetBidOrderBook()};
  EXPECT_TRUE(bidOrderBook.GetOrderBookMap().empty());

  const auto &askOrderBook{matchingEngine.GetAskOrderBook()};
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
  const auto channelInterface{std::make_shared<ChannelInterfaceMock>()};
  MatchingEngineMock matchingEngine(channelInterface);

  const boost::asio::ip::tcp::endpoint endpoint;
  OrderInsertData orderDataBid1{"mobo",
                                "ABCD",
                                {50U * std::mega::num},
                                100,
                                "Limit",
                                true,
                                std::chrono::high_resolution_clock::now(),
                                std::chrono::milliseconds::duration::zero(),
                                "id=BidOrder1",
                                "clientId=BidOrder1"};
  ASSERT_TRUE(orderDataBid1.Validate());

  OrderInsertData orderDataBid2{"mobo",
                                "ABCD",
                                {52U * std::mega::num},
                                150,
                                "Limit",
                                true,
                                std::chrono::high_resolution_clock::now(),
                                std::chrono::milliseconds::duration::zero(),
                                "id=BidOrder2",
                                "clientId=BidOrder2"};
  ASSERT_TRUE(orderDataBid2.Validate());

  OrderInsertData orderDataAsk1{"mobo",
                                "ABCD",
                                {51U * std::mega::num},
                                100,
                                "Limit",
                                false,
                                std::chrono::high_resolution_clock::now(),
                                std::chrono::milliseconds::duration::zero(),
                                "id=AskOrder1",
                                "clientId=AskOrder1"};
  ASSERT_TRUE(orderDataAsk1.Validate());

  OrderInsertData orderDataAsk2{"mobo",
                                "ABCD",
                                orderDataBid2.GetPrice(),
                                100,
                                "Limit",
                                false,
                                std::chrono::high_resolution_clock::now(),
                                std::chrono::milliseconds::duration::zero(),
                                "id=AskOrder2",
                                "clientId=AskOrder2"};
  ASSERT_TRUE(orderDataAsk2.Validate());

  //
  const OrderReply bidReply1{orderDataBid1.GetId(), orderDataBid1.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(bidReply1, endpoint));

  const OrderReply bidReply2{orderDataBid2.GetId(), orderDataBid2.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(bidReply2, endpoint));

  const OrderReply askReply1{orderDataAsk1.GetId(), orderDataAsk1.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(askReply1, endpoint));

  const OrderReply askReply2{orderDataAsk2.GetId(), orderDataAsk2.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(askReply2, endpoint));

  matchingEngine.OrderInsert(std::move(orderDataBid1), endpoint);
  matchingEngine.OrderInsert(std::move(orderDataAsk1), endpoint);
  matchingEngine.OrderInsert(std::move(orderDataAsk2), endpoint);

  // 2 trades on bid order2
  const Trade tradeBid2_1{orderDataBid2.GetAccount(),
                          orderDataBid2.GetPrice(),
                          orderDataAsk1.GetVolume(),
                          orderDataBid2.GetId(),
                          orderDataBid2.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(tradeBid2_1, endpoint));

  const Trade tradeBid2_2{orderDataBid2.GetAccount(),
                          orderDataBid2.GetPrice(),
                          orderDataAsk2.GetVolume() / 2,
                          orderDataBid2.GetId(),
                          orderDataBid2.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(tradeBid2_2, endpoint));
  // ask trades
  const Trade tradeAsk1{orderDataAsk1.GetAccount(),
                        orderDataAsk1.GetPrice(),
                        orderDataAsk1.GetVolume(),
                        orderDataAsk1.GetId(),
                        orderDataAsk1.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(tradeAsk1, endpoint));

  const Trade tradeAsk2{orderDataAsk2.GetAccount(),
                        orderDataAsk2.GetPrice(),
                        orderDataAsk2.GetVolume() / 2,
                        orderDataAsk2.GetId(),
                        orderDataAsk2.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(tradeAsk2, endpoint));

  // insert bid order 2 to fully match the ask order 1 and partially match ask order 2
  matchingEngine.OrderInsert(std::move(orderDataBid2), endpoint);
  // check the state of the orderbook
  const auto &bidOrderBook{matchingEngine.GetBidOrderBook()};
  EXPECT_EQ(bidOrderBook.GetOrderBookMap().size(), 1);
  EXPECT_EQ(bidOrderBook.GetOrderBookMap().begin()->second.GetTopLevel()->GetPrice(), orderDataBid1.GetPrice());

  const auto &askOrderBook{matchingEngine.GetAskOrderBook()};
  EXPECT_EQ(askOrderBook.GetOrderBookMap().size(), 1);
  EXPECT_EQ(askOrderBook.GetOrderBookMap().begin()->second.GetTopLevel()->GetPrice(), orderDataAsk2.GetPrice());
}

TEST_F(OrderBookTest, MultiLevelCancelOrderBidAndAskSideTest)
{
  const auto channelInterface{std::make_shared<ChannelInterfaceMock>()};
  MatchingEngineMock matchingEngine(channelInterface);

  const boost::asio::ip::tcp::endpoint endpoint;
  OrderInsertData orderDataBid1{"mobo",
                                "ABCD",
                                {50U * std::mega::num},
                                100,
                                "Limit",
                                true,
                                std::chrono::high_resolution_clock::now(),
                                std::chrono::milliseconds::duration::zero(),
                                "id=BidOrder1",
                                "clientId=BidOrder1"};
  ASSERT_TRUE(orderDataBid1.Validate());

  OrderInsertData orderDataBid2{"mobo",
                                "ABCD",
                                {52U * std::mega::num},
                                150,
                                "Limit",
                                true,
                                std::chrono::high_resolution_clock::now(),
                                std::chrono::milliseconds::duration::zero(),
                                "id=BidOrder2",
                                "clientId=BidOrder2"};
  ASSERT_TRUE(orderDataBid2.Validate());

  OrderInsertData orderDataAsk1{"mobo",
                                "ABCD",
                                {51U * std::mega::num},
                                100,
                                "Limit",
                                false,
                                std::chrono::high_resolution_clock::now(),
                                std::chrono::milliseconds::duration::zero(),
                                "id=AskOrder1",
                                "clientId=AskOrder1"};
  ASSERT_TRUE(orderDataAsk1.Validate());

  OrderInsertData orderDataAsk2{"mobo",
                                "ABCD",
                                orderDataBid2.GetPrice(),
                                100,
                                "Limit",
                                false,
                                std::chrono::high_resolution_clock::now(),
                                std::chrono::milliseconds::duration::zero(),
                                "id=AskOrder2",
                                "clientId=AskOrder2"};
  ASSERT_TRUE(orderDataAsk2.Validate());
  //
  const OrderReply bidReply1{orderDataBid1.GetId(), orderDataBid1.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(bidReply1, endpoint)).Times(2);

  const OrderReply askReply1{orderDataAsk1.GetId(), orderDataAsk1.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(askReply1, endpoint)).Times(2);

  const OrderReply askReply2{orderDataAsk2.GetId(), orderDataAsk2.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(askReply2, endpoint)).Times(2);

  matchingEngine.OrderInsert(std::move(orderDataBid1), endpoint);
  matchingEngine.OrderInsert(std::move(orderDataAsk1), endpoint);
  matchingEngine.OrderInsert(std::move(orderDataAsk2), endpoint);

  const OrderCancelData cancelBidOrder1{orderDataBid1.GetInstrument(),
                                        orderDataBid1.GetPrice(),
                                        orderDataBid1.GetIsBuySide(),
                                        orderDataBid1.GetId(),
                                        orderDataBid1.GetClientId()};
  matchingEngine.OrderCancel(cancelBidOrder1, endpoint);

  const OrderCancelData cancelAskOrder2{orderDataAsk2.GetInstrument(),
                                        orderDataAsk2.GetPrice(),
                                        orderDataAsk2.GetIsBuySide(),
                                        orderDataAsk2.GetId(),
                                        orderDataAsk2.GetClientId()};
  matchingEngine.OrderCancel(cancelAskOrder2, endpoint);

  const OrderCancelData cancelAskOrder1{orderDataAsk1.GetInstrument(),
                                        orderDataAsk1.GetPrice(),
                                        orderDataAsk1.GetIsBuySide(),
                                        orderDataAsk1.GetId(),
                                        orderDataAsk1.GetClientId()};
  matchingEngine.OrderCancel(cancelAskOrder1, endpoint);
}

TEST_F(OrderBookTest, AmendOrderVolumeTest)
{
  const auto channelInterface{std::make_shared<ChannelInterfaceMock>()};
  MatchingEngineMock matchingEngine(channelInterface);

  const boost::asio::ip::tcp::endpoint endpoint;
  OrderInsertData orderDataBid1{"mobo",
                                "ABCD",
                                {50U * std::mega::num},
                                100,
                                "Limit",
                                true,
                                std::chrono::high_resolution_clock::now(),
                                std::chrono::milliseconds::duration::zero(),
                                "id=BidOrder1",
                                "clientId=BidOrder1"};
  ASSERT_TRUE(orderDataBid1.Validate());

  OrderInsertData orderDataAsk1{"mobo",
                                "ABCD",
                                {51U * std::mega::num},
                                100,
                                "Limit",
                                false,
                                std::chrono::high_resolution_clock::now(),
                                std::chrono::milliseconds::duration::zero(),
                                "id=AskOrder1",
                                "clientId=AskOrder1"};
  ASSERT_TRUE(orderDataAsk1.Validate());

  //
  const OrderReply bidReply1{orderDataBid1.GetId(), orderDataBid1.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(bidReply1, endpoint)).Times(1);
  matchingEngine.OrderInsert(std::move(orderDataBid1), endpoint);

  const OrderReply askReply1{orderDataAsk1.GetId(), orderDataAsk1.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(askReply1, endpoint)).Times(1);
  matchingEngine.OrderInsert(std::move(orderDataAsk1), endpoint);

  // amend bid order volume
  const OrderAmendData amendBidOrder1{orderDataBid1.GetAccount(),
                                      orderDataBid1.GetInstrument(),
                                      orderDataBid1.GetPrice(),
                                      orderDataBid1.GetPrice(),
                                      orderDataBid1.GetVolume(),
                                      orderDataBid1.GetVolume() + 100,
                                      orderDataBid1.GetType(),
                                      orderDataBid1.GetIsBuySide(),
                                      std::chrono::high_resolution_clock::now(),
                                      std::chrono::milliseconds::duration::zero(),
                                      orderDataBid1.GetId(),
                                      "ClientId=AmendBidOrder1"};

  const OrderReply amendBidReply1{orderDataBid1.GetId(), amendBidOrder1.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(amendBidReply1, endpoint)).Times(1);

  matchingEngine.OrderAmend(amendBidOrder1, endpoint);
  EXPECT_EQ(amendBidOrder1.GetNewVolume(),
            matchingEngine.GetBidOrderBook().GetOrderBookMap().begin()->second.GetTopLevel()->GetVolume());

  // amend ask order volume
  const OrderAmendData amendAskOrder1{orderDataAsk1.GetAccount(),
                                      orderDataAsk1.GetInstrument(),
                                      orderDataAsk1.GetPrice(),
                                      orderDataAsk1.GetPrice(),
                                      orderDataAsk1.GetVolume(),
                                      orderDataAsk1.GetVolume() / 2,   // new volume
                                      orderDataAsk1.GetType(),
                                      orderDataAsk1.GetIsBuySide(),
                                      std::chrono::high_resolution_clock::now(),
                                      std::chrono::milliseconds::duration::zero(),
                                      orderDataAsk1.GetId(),
                                      "ClientId=AmendAskOrder1"};

  const OrderReply amendAskReply1{orderDataAsk1.GetId(), amendAskOrder1.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(amendAskReply1, endpoint)).Times(1);

  matchingEngine.OrderAmend(amendAskOrder1, endpoint);
  EXPECT_EQ(amendAskOrder1.GetNewVolume(),
            matchingEngine.GetAskOrderBook().GetOrderBookMap().begin()->second.GetTopLevel()->GetVolume());
  // amend ask order volume to zero
  const OrderAmendData amendAskOrder2{orderDataAsk1.GetAccount(),
                                      orderDataAsk1.GetInstrument(),
                                      orderDataAsk1.GetPrice(),
                                      orderDataAsk1.GetPrice(),
                                      orderDataAsk1.GetVolume(),
                                      0,   // new volume
                                      orderDataAsk1.GetType(),
                                      orderDataAsk1.GetIsBuySide(),
                                      std::chrono::high_resolution_clock::now(),
                                      std::chrono::milliseconds::duration::zero(),
                                      orderDataAsk1.GetId(),
                                      "ClientId=AmendAskOrder2VolumeZero"};

  const OrderReply amendAskReply2{orderDataAsk1.GetId(), amendAskOrder2.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(amendAskReply2, endpoint)).Times(1);

  matchingEngine.OrderAmend(amendAskOrder2, endpoint);
  EXPECT_TRUE(matchingEngine.GetAskOrderBook().GetOrderBookMap().empty());
}

TEST_F(OrderBookTest, AmendAskOrderPriceVolumeTest)
{
  const auto channelInterface{std::make_shared<ChannelInterfaceMock>()};
  MatchingEngineMock matchingEngine(channelInterface);

  const boost::asio::ip::tcp::endpoint endpoint;

  OrderInsertData orderDataAsk1{"mobo",
                                "ABCD",
                                {51U * std::mega::num},
                                100,
                                "Limit",
                                false,
                                std::chrono::high_resolution_clock::now(),
                                std::chrono::milliseconds::duration::zero(),
                                "id=AskOrder1",
                                "clientId=AskOrder1"};
  ASSERT_TRUE(orderDataAsk1.Validate());

  const OrderReply askReply1{orderDataAsk1.GetId(), orderDataAsk1.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(askReply1, endpoint)).Times(1);
  matchingEngine.OrderInsert(std::move(orderDataAsk1), endpoint);

  // amend ask order volume and price
  const auto newAskVolume1{orderDataAsk1.GetVolume() / 2};
  const auto newAskPrice1{orderDataAsk1.GetPrice() + (orderDataAsk1.GetPrice() / 2)};
  const OrderAmendData amendAskOrder1{orderDataAsk1.GetAccount(),
                                      orderDataAsk1.GetInstrument(),
                                      orderDataAsk1.GetPrice(),
                                      newAskPrice1,
                                      orderDataAsk1.GetVolume(),
                                      newAskVolume1,   // new volume
                                      orderDataAsk1.GetType(),
                                      orderDataAsk1.GetIsBuySide(),
                                      std::chrono::high_resolution_clock::now(),
                                      std::chrono::milliseconds::duration::zero(),
                                      orderDataAsk1.GetId(),
                                      "ClientId=AmendAskOrder1"};

  const OrderReply amendAskReply1{orderDataAsk1.GetId(), amendAskOrder1.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(amendAskReply1, endpoint)).Times(1);

  matchingEngine.OrderAmend(amendAskOrder1, endpoint);
  EXPECT_EQ(newAskVolume1, matchingEngine.GetAskOrderBook().GetOrderBookMap().begin()->second.GetTopLevel()->GetVolume());
  EXPECT_EQ(newAskPrice1, matchingEngine.GetAskOrderBook().GetOrderBookMap().begin()->second.GetTopLevel()->GetPrice());
}

TEST_F(OrderBookTest, AmendAskOrderPriceZeroVolumeTest)
{
  const auto channelInterface{std::make_shared<ChannelInterfaceMock>()};
  MatchingEngineMock matchingEngine(channelInterface);

  const boost::asio::ip::tcp::endpoint endpoint;

  OrderInsertData orderDataAsk1{"mobo",
                                "ABCD",
                                {51U * std::mega::num},
                                100,
                                "Limit",
                                false,
                                std::chrono::high_resolution_clock::now(),
                                std::chrono::milliseconds::duration::zero(),
                                "id=AskOrder1",
                                "clientId=AskOrder1"};
  ASSERT_TRUE(orderDataAsk1.Validate());

  const OrderReply askReply1{orderDataAsk1.GetId(), orderDataAsk1.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(askReply1, endpoint)).Times(1);
  matchingEngine.OrderInsert(std::move(orderDataAsk1), endpoint);

  // amend ask order volume to zero and price
  const auto zeroVolume{0};
  const auto newAskPrice1{orderDataAsk1.GetPrice() + (orderDataAsk1.GetPrice() / 2)};
  const OrderAmendData amendAskOrder1{orderDataAsk1.GetAccount(),
                                      orderDataAsk1.GetInstrument(),
                                      orderDataAsk1.GetPrice(),
                                      newAskPrice1,
                                      orderDataAsk1.GetVolume(),
                                      zeroVolume,   // new volume
                                      orderDataAsk1.GetType(),
                                      orderDataAsk1.GetIsBuySide(),
                                      std::chrono::high_resolution_clock::now(),
                                      std::chrono::milliseconds::duration::zero(),
                                      orderDataAsk1.GetId(),
                                      "ClientId=AmendAskOrder1"};

  const OrderReply amendAskReply1{orderDataAsk1.GetId(), amendAskOrder1.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(amendAskReply1, endpoint)).Times(1);

  matchingEngine.OrderAmend(amendAskOrder1, endpoint);
  EXPECT_TRUE(matchingEngine.GetAskOrderBook().GetOrderBookMap().empty());
}

TEST_F(OrderBookTest, AmendBidOrderPriceVolumeTest)
{
  const auto channelInterface{std::make_shared<ChannelInterfaceMock>()};
  MatchingEngineMock matchingEngine(channelInterface);

  const boost::asio::ip::tcp::endpoint endpoint;
  OrderInsertData orderDataBid1{"mobo",
                                "ABCD",
                                {50U * std::mega::num},
                                100,
                                "Limit",
                                true,
                                std::chrono::high_resolution_clock::now(),
                                std::chrono::milliseconds::duration::zero(),
                                "id=BidOrder1",
                                "clientId=BidOrder1"};
  ASSERT_TRUE(orderDataBid1.Validate());
  //
  const OrderReply bidReply1{orderDataBid1.GetId(), orderDataBid1.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(bidReply1, endpoint)).Times(1);
  matchingEngine.OrderInsert(std::move(orderDataBid1), endpoint);

  {   // amend bid order volume
    const auto newBidVolume1{orderDataBid1.GetVolume() + 100};
    const auto newBidPrice1{orderDataBid1.GetPrice() / 2};
    const OrderAmendData amendBidOrder1{orderDataBid1.GetAccount(),
                                        orderDataBid1.GetInstrument(),
                                        orderDataBid1.GetPrice(),
                                        newBidPrice1,
                                        orderDataBid1.GetVolume(),
                                        newBidVolume1,
                                        orderDataBid1.GetType(),
                                        orderDataBid1.GetIsBuySide(),
                                        std::chrono::high_resolution_clock::now(),
                                        std::chrono::milliseconds::duration::zero(),
                                        orderDataBid1.GetId(),
                                        "ClientId=AmendBidOrder1"};

    const OrderReply amendBidReply1{orderDataBid1.GetId(), amendBidOrder1.GetClientId()};
    EXPECT_CALL(matchingEngine, CreateAndSendMessage(amendBidReply1, endpoint)).Times(1);
    EXPECT_TRUE(amendBidOrder1.Validate());

    matchingEngine.OrderAmend(amendBidOrder1, endpoint);
    EXPECT_EQ(newBidVolume1, matchingEngine.GetBidOrderBook().GetOrderBookMap().begin()->second.GetTopLevel()->GetVolume());

    EXPECT_EQ(newBidPrice1, matchingEngine.GetBidOrderBook().GetOrderBookMap().begin()->second.GetTopLevel()->GetPrice());
  }
}

TEST_F(OrderBookTest, AmendAskOrderMatchBidPriceTest)
{
  const auto channelInterface{std::make_shared<ChannelInterfaceMock>()};
  MatchingEngineMock matchingEngine(channelInterface);

  const boost::asio::ip::tcp::endpoint endpoint;
  OrderInsertData orderDataBid1{"mobo",
                                "ABCD",
                                {50U * std::mega::num},
                                100,
                                "Limit",
                                true,
                                std::chrono::high_resolution_clock::now(),
                                std::chrono::milliseconds::duration::zero(),
                                "id=BidOrder1",
                                "clientId=BidOrder1"};
  ASSERT_TRUE(orderDataBid1.Validate());
  //
  const OrderReply bidReply1{orderDataBid1.GetId(), orderDataBid1.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(bidReply1, endpoint)).Times(1);
  matchingEngine.OrderInsert(std::move(orderDataBid1), endpoint);

  // insert ask order
  OrderInsertData orderDataAsk1{"mobo",
                                "ABCD",
                                {51U * std::mega::num},
                                100,
                                "Limit",
                                false,
                                std::chrono::high_resolution_clock::now(),
                                std::chrono::milliseconds::duration::zero(),
                                "id=AskdOrder1",
                                "clientId=AskOrder1"};
  ASSERT_TRUE(orderDataAsk1.Validate());
  //
  const OrderReply askReply1{orderDataAsk1.GetId(), orderDataAsk1.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(askReply1, endpoint));
  matchingEngine.OrderInsert(std::move(orderDataAsk1), endpoint);

  // amend bid order to match the ask order
  const auto newAskVolume1{orderDataBid1.GetVolume()};
  const auto newAskPrice1{orderDataBid1.GetPrice()};
  const OrderAmendData amendAskOrder1{orderDataAsk1.GetAccount(),
                                      orderDataAsk1.GetInstrument(),
                                      orderDataAsk1.GetPrice(),
                                      newAskPrice1,
                                      orderDataAsk1.GetVolume(),
                                      newAskVolume1,
                                      orderDataAsk1.GetType(),
                                      orderDataAsk1.GetIsBuySide(),
                                      std::chrono::high_resolution_clock::now(),
                                      std::chrono::milliseconds::duration::zero(),
                                      orderDataAsk1.GetId(),
                                      "ClientId=AmendAskOrder1"};

  const OrderReply amendAskReply1{orderDataAsk1.GetId(), amendAskOrder1.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(amendAskReply1, endpoint));

  // create 2 trades expectations
  const Trade tradeBid{orderDataBid1.GetAccount(),
                       orderDataBid1.GetPrice(),
                       orderDataBid1.GetVolume(),
                       orderDataBid1.GetId(),
                       orderDataBid1.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(tradeBid, endpoint));

  const Trade tradeAsk{orderDataAsk1.GetAccount(),
                       orderDataBid1.GetPrice(),
                       orderDataAsk1.GetVolume(),
                       orderDataAsk1.GetId(),
                       orderDataAsk1.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(tradeAsk, endpoint));

  EXPECT_TRUE(amendAskOrder1.Validate());
  matchingEngine.OrderAmend(amendAskOrder1, endpoint);
}

TEST_F(OrderBookTest, AmendBidOrderMatchAskPriceTest)
{
  const auto channelInterface{std::make_shared<ChannelInterfaceMock>()};
  MatchingEngineMock matchingEngine(channelInterface);

  const boost::asio::ip::tcp::endpoint endpoint;
  OrderInsertData orderDataBid1{"mobo",
                                "ABCD",
                                {50U * std::mega::num},
                                100,
                                "Limit",
                                true,
                                std::chrono::high_resolution_clock::now(),
                                std::chrono::milliseconds::duration::zero(),
                                "id=BidOrder1",
                                "clientId=BidOrder1"};
  ASSERT_TRUE(orderDataBid1.Validate());
  //
  const OrderReply bidReply1{orderDataBid1.GetId(), orderDataBid1.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(bidReply1, endpoint)).Times(1);
  matchingEngine.OrderInsert(std::move(orderDataBid1), endpoint);

  // insert ask order
  OrderInsertData orderDataAsk1{"mobo",
                                "ABCD",
                                {51U * std::mega::num},
                                100,
                                "Limit",
                                false,
                                std::chrono::high_resolution_clock::now(),
                                std::chrono::milliseconds::duration::zero(),
                                "id=AskdOrder1",
                                "clientId=AskOrder1"};
  ASSERT_TRUE(orderDataAsk1.Validate());
  //
  const OrderReply askReply1{orderDataAsk1.GetId(), orderDataAsk1.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(askReply1, endpoint));
  matchingEngine.OrderInsert(std::move(orderDataAsk1), endpoint);

  // amend bid order to match the ask order
  const auto newBidVolume1{orderDataAsk1.GetVolume()};
  const auto newBidPrice1{orderDataAsk1.GetPrice()};
  const OrderAmendData amendBidOrder1{orderDataBid1.GetAccount(),
                                      orderDataBid1.GetInstrument(),
                                      orderDataBid1.GetPrice(),
                                      newBidPrice1,
                                      orderDataBid1.GetVolume(),
                                      newBidVolume1,
                                      orderDataBid1.GetType(),
                                      orderDataBid1.GetIsBuySide(),
                                      std::chrono::high_resolution_clock::now(),
                                      std::chrono::milliseconds::duration::zero(),
                                      orderDataBid1.GetId(),
                                      "ClientId=AmendBidOrder1"};

  const OrderReply amendBidReply1{orderDataBid1.GetId(), amendBidOrder1.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(amendBidReply1, endpoint));

  // create 2 trades expectations
  const Trade tradeBid{orderDataBid1.GetAccount(),
                       orderDataAsk1.GetPrice(),
                       orderDataBid1.GetVolume(),
                       orderDataBid1.GetId(),
                       orderDataBid1.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(tradeBid, endpoint));

  const Trade tradeAsk{orderDataAsk1.GetAccount(),
                       orderDataAsk1.GetPrice(),
                       orderDataAsk1.GetVolume(),
                       orderDataAsk1.GetId(),
                       orderDataAsk1.GetClientId()};
  EXPECT_CALL(matchingEngine, CreateAndSendMessage(tradeAsk, endpoint));

  EXPECT_TRUE(amendBidOrder1.Validate());
  matchingEngine.OrderAmend(amendBidOrder1, endpoint);
}