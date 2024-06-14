#include <piconaut/piconaut.h>

using namespace piconaut;
class MyHandler : public handlers::HandlerBase {
 public:
  void Handle(const http::Request& req,
              const http::Response& res) const override {
    
    res.SendJson("{\"msg\":\"Hello World from piconaut!\"}", 200);
    std::cout << "Helloworld sent" << std::endl;
  }
};

int main() {

  const int num_threads = 1;
  const int port = 9066;
  const std::string host = "0.0.0.0";  // Can be any IP address or hostname

  try {
    http::H2OServer server(host, port);

    std::shared_ptr<handlers::HandlerBase> handler1 =
        std::make_shared<MyHandler>();
    std::shared_ptr<handlers::HandlerBase> handler2 =
        std::make_shared<MyHandler>();

    // Register handlers for different paths
    server.RegisterHandler("/", handler1);  // Handle root path
    server.RegisterHandler("/hello", handler2);  // Handle another path
    server.Start();
    // server.Wait();
    // The server will run indefinitely until stopped
    // To stop the server, you can call server.stop() from another thread or
    // signal handler

  } catch (const std::exception& ex) {
    std::cerr << "Error: " << ex.what() << std::endl;
    return 1;
  }

  return 0;
}