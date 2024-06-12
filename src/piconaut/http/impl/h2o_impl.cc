#include "piconaut/http/impl/h2o_impl.h"

PICONAUT_INNER_NAMESPACE(http)

namespace impl {

static std::vector<H2OServerImpl*> g_server_;

void SignalHandler(int signal) {
  if (signal == SIGINT || signal == SIGTERM) {
    for (auto& server : g_server_) {
      server->Stop();
    }
  }
}

}  // namespace impl

PICONAUT_INNER_END_NAMESPACE