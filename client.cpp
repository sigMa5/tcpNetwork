#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
using tcp  = boost::asio::ip::tcp;
using json = nlohmann::json;

int main(){
  boost::asio::io_context ctx;
  tcp::socket sock(ctx);
  sock.connect({{}, 4000}, {{}, "192.168.1.100"}); // server IP

  std::thread reader([&]{
    boost::asio::streambuf buf;
    while (boost::asio::read_until(sock, buf, '\n')){
      std::istream is(&buf);
      std::string line; std::getline(is, line);
      auto pkt = json::parse(line);
      std::cout << "RX: " << pkt.dump() << "\n";
    }
  });

  // send commands:
  for (auto cmd : {"send_odd\n", "send_even\n"}){
    boost::asio::write(sock, boost::asio::buffer(cmd));
    std::this_thread::sleep_for(std::chrono::seconds(5));
  }
  boost::asio::write(sock, boost::asio::buffer("exit\n"));
  reader.join();
}
