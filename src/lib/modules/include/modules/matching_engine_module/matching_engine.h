#pragma once
#include "common/channel_interface.h"
#include "modules/matching_engine_module/i_order_handler.h"
#include "modules/matching_engine_module/order_book.h"
#include <map>

namespace moboware::modules {

class MatchingEngine
{
public:
  explicit MatchingEngine(const std::shared_ptr<common::ChannelInterface>& channelInterface);
  MatchingEngine(const MatchingEngine&) = delete;
  MatchingEngine(MatchingEngine&&) = delete;
  MatchingEngine& operator=(const MatchingEngine&) = delete;
  MatchingEngine& operator=(MatchingEngine&&) = delete;
  virtual ~MatchingEngine() = default;

  void OrderInsert(const OrderInsertData& orderInsert, const boost::asio::ip::tcp::endpoint& endpoint);
  void OrderAmend(const OrderAmendData& orderCancel, const boost::asio::ip::tcp::endpoint& endpoint);

  void OrderCancel(const OrderCancelData& orderCancel, const boost::asio::ip::tcp::endpoint& endpoint);

  [[nodiscard]] const OrderBidBook_t& GetBidOrderBook() const { return m_Bids; }
  [[nodiscard]] const OrderAskBook_t& GetAskOrderBook() const { return m_Asks; }

  void GetOrderBook(const boost::asio::ip::tcp::endpoint& endpoint);

protected:
  virtual void CreateAndSendMessage(const OrderReply& orderInsertReply, const boost::asio::ip::tcp::endpoint& endpoint);
  virtual void CreateAndSendMessage(const ErrorReply& errorReply, const boost::asio::ip::tcp::endpoint& endpoint);
  virtual void CreateAndSendMessage(const Trade& trade, const boost::asio::ip::tcp::endpoint& endpoint);

private:
  template<typename TOrderBook1, typename TOrderBook2>
  void ExecuteOrder(TOrderBook1& orderBook, TOrderBook2& otherSideOrderBook, const boost::asio::ip::tcp::endpoint& endpoint);

  std::mutex m_Mutex;

  const std::shared_ptr<common::ChannelInterface> m_ChannelInterface;

  OrderBidBook_t m_Bids; // the order bids are descending sorted
  OrderAskBook_t m_Asks; // the asks are ascending sorted
};
}