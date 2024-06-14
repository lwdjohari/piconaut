#include <piconaut/piconaut.h>

#include <memory>

using namespace piconaut;

std::shared_ptr<http::H2OServer> g_server_;
std::shared_ptr<sys::SignalHandler> g_signal_;


void CaptureSignalDefault(sys::SignalHandler* handler, sys::SignalType signum) {
  switch (signum) {
    case sys::SignalType::SIGINT_SIGNAL:
    case sys::SignalType::SIGTERM_SIGNAL:
      std::cout << "\nServer stopped (" << static_cast<int>(signum) << ")" << std::endl;
      if (!g_server_)
        return;
      g_server_->Stop();
      break;
    default:
      break;
  }
}

class HelloWorldHandler : public handlers::HandlerBase {
 public:
  void Handle(const http::Request& req,
              const http::Response& res) const override {

    formats::json::Value json;
    json["msg"] = "Hello world";
    json["server"] = "Piconaut Framework";
    json["version"] = "v0.2.1";
  
    res.SendJson(json, 200);
    std::cout << "Helloworld sent" << std::endl;
  }
};


int main() {
  const int num_threads = 1;
  const int port = 9066;
  const std::string host = "0.0.0.0";  // Can be any IP address or hostname

  try {
    g_signal_ = std::make_shared<sys::SignalHandler>();
    g_signal_->RegisterCallback(CaptureSignalDefault);
    g_signal_->Start();

    g_server_ = std::make_shared<http::H2OServer>(host, port);

    std::shared_ptr<handlers::HandlerBase> handler1 =
        std::make_shared<HelloWorldHandler>();
    std::shared_ptr<handlers::HandlerBase> handler2 =
        std::make_shared<HelloWorldHandler>();

    // Register handlers for different paths
    g_server_->RegisterHandler("/", handler1);       // Handle root path
    g_server_->RegisterHandler("/hello", handler2);  // Handle another path
    g_server_->Start();
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