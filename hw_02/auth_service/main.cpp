#include "http_auth_server.h"

int main(int argc, char* argv[]) {
  HTTPWebServer app;
  return app.run(argc, argv);
}
