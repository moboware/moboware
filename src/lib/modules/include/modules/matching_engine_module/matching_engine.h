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

protected:
private:
  bool Insert(const OrderData& orderInsert);

  void CreateAndSendMessage(const OrderInsertReply& orderInsertReply, const boost::asio::ip::tcp::endpoint& endpoint);
  void CreateAndSendMessage(const ErrorReply& errorReply, const boost::asio::ip::tcp::endpoint& endpoint);
  void CreateAndSendMessage(const Trade& trade, const boost::asio::ip::tcp::endpoint& endpoint);

  void CheckMatch(const boost::asio::ip::tcp::endpoint& endpoint);

  std::mutex m_Mutex;

  const std::shared_ptr<common::ChannelInterface> m_ChannelInterface;

  OrderBook<std::greater<uint64_t>> m_Bids; // the order bids are descending sorted
  OrderBook<std::less<uint64_t>> m_Asks;    // the asks are ascending sorted
};
}