#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <nlohmann/json.hpp>
#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <set>
#include <thread>
#include <vector>

using tcp   = boost::asio::ip::tcp;
using json  = nlohmann::json;
namespace chrono = std::chrono;

class Session : public std::enable_shared_from_this<Session> {
  tcp::socket socket_;
  boost::asio::steady_timer timer_;
  int mode_ = 0; // 0=idle,1=odd,2=even

public:
  Session(tcp::socket sock, boost::asio::io_context& ctx)
    : socket_(std::move(sock))
    , timer_(ctx)
  {}

  void start() {
    read_command();
    schedule_send();
  }

private:
  void read_command() {
    auto self = shared_from_this();
    boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(tmp_), '\n',
      [this, self](auto ec, std::size_t length){
        if (ec) return; // connection closed
        std::string line = tmp_.substr(0, length-1);
        tmp_.erase(0, length);
        if (line == "send_odd")  mode_ = 1;
        if (line == "send_even") mode_ = 2;
        if (line == "exit")      return socket_.close();
        read_command();
      });
  }

  void schedule_send() {
    auto self = shared_from_this();
    timer_.expires_after(chrono::milliseconds(200));
    timer_.async_wait([this, self](auto){
      if (!socket_.is_open()) return;
      if (mode_ != 0) {
        static thread_local std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<int> dist(0,1000);
        int raw = dist(rng);
        int number = (mode_==1 ? raw*2+1 : raw*2);
        json pkt = {
          {"number", number},
          {"recent_command", mode_==1 ? "send_odd" : "send_even"},
          {"state", mode_==1 ? "sending_odds" : "sending_evens"}
        };
        auto msg = pkt.dump() + "\n";
        boost::asio::async_write(socket_, boost::asio::buffer(msg),
          [this, self](auto, auto){});
      }
      schedule_send();
    });
  }

  std::string tmp_;
};

class Server {
  boost::asio::io_context& ioctx_;
  tcp::acceptor acceptor_;

public:
  Server(boost::asio::io_context& ctx, tcp::endpoint ep)
    : ioctx_(ctx)
    , acceptor_(ctx, ep)
  {
    do_accept();
  }

private:
  void do_accept() {
    acceptor_.async_accept(
      [this](auto ec, tcp::socket sock){
        if (!ec) {
          std::make_shared<Session>(std::move(sock), ioctx_)->start();
        }
        do_accept();
      });
  }
};

int main(int argc, char* argv[]){
#ifdef _WIN32
  WSADATA wsa;
  WSAStartup(MAKEWORD(2,2), &wsa);
#endif

  const int port = 4000;
  boost::asio::io_context ioctx{std::thread::hardware_concurrency()};

  Server server(ioctx, tcp::endpoint(tcp::v4(), port));
  std::cout << "Listening on port " << port << "\n";

  // run io_context on a pool of threads
  std::vector<std::thread> pool;
  for (int i = 0; i < std::thread::hardware_concurrency(); ++i)
    pool.emplace_back([&]{ ioctx.run(); });

  for (auto& t : pool) t.join();

#ifdef _WIN32
  WSACleanup();
#endif
  return 0;
}
