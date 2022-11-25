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

  void OrderInsert(const OrderData& orderInsert, const boost::asio::ip::tcp::endpoint& endpoint);
  const OrderBidBook_t& GetBidOrderBook() const { return m_Bids; }
  const OrderAskBook_t& GetAskOrderBook() const { return m_Asks; }

protected:
  virtual void CreateAndSendMessage(const OrderInsertReply& orderInsertReply, const boost::asio::ip::tcp::endpoint& endpoint);
  virtual void CreateAndSendMessage(const ErrorReply& errorReply, const boost::asio::ip::tcp::endpoint& endpoint);
  virtual void CreateAndSendMessage(const Trade& trade, const boost::asio::ip::tcp::endpoint& endpoint);

private:
  bool Insert(const OrderData& orderInsert);

  void CheckMatch(const OrderData& newOrder, const boost::asio::ip::tcp::endpoint& endpoint);

  std::mutex m_Mutex;

  const std::shared_ptr<common::ChannelInterface> m_ChannelInterface;

  OrderBidBook_t m_Bids; // the order bids are descending sorted
  OrderAskBook_t m_Asks; // the asks are ascending sorted
};
}