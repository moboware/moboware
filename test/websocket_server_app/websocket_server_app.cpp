#include <boost/beast/websocket.hpp>
#include <boost/beast/core.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <map>
#include "common/service.h"
#include "common/timer.h"
#include "common/log.h"

using namespace boost;
using namespace boost::beast;
using namespace boost::asio;
using namespace boost::asio::ip;

namespace moboware {

  class Session
  {
  public:
    explicit Session(const std::shared_ptr<moboware::common::Service>& service,
      tcp::socket&& webSocket);
    void Start();
    inline bool IsOpen() const { return m_WebSocket.is_open(); }
  private:
    void ReadData();
    const std::shared_ptr<moboware::common::Service> m_Service;
    websocket::stream<beast::tcp_stream> m_WebSocket;
    beast::flat_buffer m_ReadBuffer;
  };

  /**
   * @brief
   *
   */
  class WebsocketServer
  {
  public:
    explicit WebsocketServer(const std::shared_ptr<moboware::common::Service>& service);
    WebsocketServer(const WebsocketServer&);
    WebsocketServer(WebsocketServer&&);
    WebsocketServer& operator = (const WebsocketServer&);
    WebsocketServer& operator = (WebsocketServer&&);
    ~WebsocketServer() = default;

    [[nodiscard]] bool Start(const std::string address, const short port);

  private:
    void Accept();
    void StartSessionTimer();
    std::size_t CheckClosedSessions();

    const std::shared_ptr<moboware::common::Service> m_Service;
    boost::asio::ip::tcp::acceptor m_Acceptor;

    using Sessions_t = std::map < std::pair<boost::asio::ip::address, boost::asio::ip::port_type>, std::shared_ptr<moboware::Session>>;
    Sessions_t m_Sessions;
    common::Timer m_sessionTimer;
  };
}

using namespace moboware;


Session::Session(const std::shared_ptr<moboware::common::Service>& service,
  tcp::socket&& webSocket)
  :
  m_Service(service),
  m_WebSocket(std::move(webSocket)) {
}

void Session::Start()
{
  // Get on the correct executor
  // We need to be executing within a strand to perform async operations
  // on the I/O objects in this session. Although not strictly necessary
  // for single-threaded contexts, this example code is written to be
  // thread-safe by default.

  const auto runFunc = [this]()
  {
    // Set suggested timeout settings for the websocket
    m_WebSocket.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));

    // Set a decorator to change the Server of the handshake
    m_WebSocket.set_option(websocket::stream_base::decorator(
      [](websocket::response_type& res) {
        res.set(http::field::server, std::string(BOOST_BEAST_VERSION_STRING) + " websocket-server-async");
      }));

    {
      // Accept the websocket handshake
      const auto acceptHandshakeFn = [this](const beast::error_code& ec)
      {
        if (ec)
        {
          LOG("Failed to accept web socket handshake " << ec);
          return;
        }

        ReadData();
      };

      m_WebSocket.async_accept(beast::bind_front_handler(acceptHandshakeFn));
    }
  };

  asio::dispatch(m_WebSocket.get_executor(), beast::bind_front_handler(runFunc));
}

void Session::ReadData()
{

  // clear read buffer
  m_ReadBuffer.clear();

  const auto readDataFunc = [this](const beast::error_code& ec, const std::size_t bytesTransferred)
  {

    boost::ignore_unused(bytesTransferred);

    // This indicates that the session was closed
    if (ec == websocket::error::closed)
    {
      LOG("Web socket closed");
      return;
    }

    if (ec)
    {
      LOG("Read error:" << ec << ", open:" << std::boolalpha << m_WebSocket.is_open());
      return;
    }

    LOG("Read data:" << std::string((const char*)m_ReadBuffer.data().data(), m_ReadBuffer.data().size()));
    // forward read data to channel


    // initialize new read operation
    this->ReadData();
  };

  m_WebSocket.async_read(m_ReadBuffer, beast::bind_front_handler(readDataFunc));
}

////////////////////////////////////////////////
WebsocketServer::WebsocketServer(const std::shared_ptr<moboware::common::Service>& service)
  :
  m_Service(service),
  m_Acceptor(service->GetIoService()),
  m_sessionTimer(service)
{

}

[[nodiscard]] bool WebsocketServer::Start(const std::string address, const short port)
{
  // create end point
  const ip::tcp::endpoint endpoint(ip::make_address(address), port);
  //
  beast::error_code ec;

  // Open the acceptor
  m_Acceptor.open(endpoint.protocol(), ec);
  if (ec)
  {
    LOG("Open acceptor failed:" << ec);
    return false;
  }

  // Allow address reuse
  m_Acceptor.set_option(asio::socket_base::reuse_address(true), ec);
  if (ec)
  {
    LOG("set_option failed:" << ec);
    return false;
  }

  // Bind to the server address
  m_Acceptor.bind(endpoint, ec);
  if (ec)
  {
    LOG("bind failed:" << ec);
    return false;
  }

  // Start listening for connections
  m_Acceptor.listen(asio::socket_base::max_listen_connections, ec);
  if (ec)
  {
    LOG("start listen failed:" << ec);
    return false;
  }

  Accept();

  StartSessionTimer();
  return true;
}

void WebsocketServer::StartSessionTimer()
{
  const auto timerFunc = [this](common::Timer& timer)
  {
    if (int removedSessions = this->CheckClosedSessions())
    {
      LOG("Removed sessions:" << removedSessions);
    }
    timer.Restart();
  };

  m_sessionTimer.Start(timerFunc, std::chrono::seconds(5));
}

std::size_t WebsocketServer::CheckClosedSessions()
{
  int numberOfSessions{};

  auto iter = m_Sessions.begin();
  while (iter != m_Sessions.end())
  {
    const auto session = iter->second;
    if (not session->IsOpen())
    {
      iter = m_Sessions.erase(iter);
      numberOfSessions++;
    }
    else {
      iter++;
    }
  }
  return numberOfSessions;
}

void WebsocketServer::Accept()
{
  const auto acceptorFunc = [this](beast::error_code ec, tcp::socket webSocket) //
  {
    if (ec)
    {
      LOG("Failed to accept connection:" << ec);
    }
    else {
      LOG("Connection accepted from " << webSocket.remote_endpoint().address().to_string() << ":" << webSocket.remote_endpoint().port());

      // create session and store in our session list
      const auto key = std::make_pair(webSocket.remote_endpoint().address(), webSocket.remote_endpoint().port());
      const auto session = std::make_shared<Session>(this->m_Service, std::move(webSocket));

      m_Sessions[key] = session;

      session->Start();
    }
    Accept();
  };

  // The new connection gets its own strand
  m_Acceptor.async_accept(asio::make_strand(m_Service->GetIoService()), beast::bind_front_handler(acceptorFunc));
}

int main(const int, const char* [])
{
  const auto service = std::make_shared<moboware::common::Service>();
  moboware::WebsocketServer websocketServer(service);

  if (not websocketServer.Start("0.0.0.0", 8080))
  {
    return EXIT_FAILURE;
  }
  LOG("Running waiting for connections");

  service->Run();
  //
  return 0;
}