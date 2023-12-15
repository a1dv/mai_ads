#include "http_route_server.h"

int main(int argc, char* argv[]) {
  HTTPRouteServer app;
  return app.run(argc, argv);
}
