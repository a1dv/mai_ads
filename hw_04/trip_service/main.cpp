#include "http_trip_server.h"

int main(int argc, char* argv[]) {
    HTTPTripServer app;
    return app.run(argc, argv);
}
