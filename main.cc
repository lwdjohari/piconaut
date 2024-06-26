#include <piconaut/piconaut.h>

#include <memory>

using namespace piconaut;

std::shared_ptr<http::H2OServer> g_server_;
std::shared_ptr<sys::SignalHandler> g_signal_;

void CaptureSignalDefault(sys::SignalHandler* handler, sys::SignalType signum) {
  switch (signum) {
    case sys::SignalType::SIGINT_SIGNAL:
    case sys::SignalType::SIGTERM_SIGNAL:
      std::cout << "\nServer stopped (" << static_cast<int>(signum) << ")"
                << std::endl;
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
  void HandleRequest(const http::Request& req, const http::Response& res,
                     const std::unordered_map<std::string, std::string>& params)
      const override {
    
    auto user_agent = req.GetHeader("user-agent");
    auto accept_encoding = req.GetHeader("accept-encoding");
    auto accept_lang = req.GetHeader("accept-language");
    auto cache_control = req.GetHeader("cache-control");
    auto pragma = req.GetHeader("pragma");

    formats::json::ValueBuilder json;
    std::cout << "Path: " << req.GetPath() << std::endl;

    json["server"].CreateJsonObject();
    json["server"]["name"] = "Piconaut Framework";
    json["server"]["version"] = "v0.2.1";
    json["status"] = 200;
    json["userAgent"] = user_agent;
    json["acceptLanguage"] = accept_lang;
    json["acceptEncoding"] = accept_encoding;
    json["cacheControl"] = cache_control;
    json["pragma"] = pragma;

    if(params.size()!=0){
      json["params"].CreateJsonObject();
    }
    for (auto &p : params)
    {
      json["params"][p.first] = p.second;
    }
    
    res.SendJson(json.SerializeToBytes(), 200);
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
        std::shared_ptr<handlers::HandlerBase> handler3 =
        std::make_shared<HelloWorldHandler>();

    // Register handlers for different paths
    g_server_->RegisterHandler("/post", handler1); 
    g_server_->RegisterHandler("/post/{id}", handler2);  
    g_server_->RegisterHandler("/post/{id}/{slug}", handler3);  
    g_server_->RegisterHandler("/{year}/{month}", handler3);
    g_server_->RegisterHandler("/{year}/{month}/post/{id}/{slug}", handler3);

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