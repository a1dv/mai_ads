#ifndef HTTPROUTEREQUESTFACTORY_H
#define HTTPROUTEREQUESTFACTORY_H

#include <iostream>
#include <vector>
#include <string>
#include "Poco/DateTimeFormat.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Exception.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/ThreadPool.h"
#include "Poco/Timestamp.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/ServerApplication.h"

using Poco::DateTimeFormat;
using Poco::DateTimeFormatter;
using Poco::ThreadPool;
using Poco::Timestamp;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerParams;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::ServerSocket;
using Poco::Util::Application;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionCallback;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;

#include "handlers/route_handler.h"
#include "../helper.h"

class HTTPRequestFactory : public HTTPRequestHandlerFactory {
 public:
  HTTPRequestFactory(const std::string& format) : _format(format) {}

  HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request) {
    const std::vector<std::string> allowed_paths{ "/add", "/search" };

    std::cout << "request:" << request.getURI() << std::endl;
    for (const std::string& path : allowed_paths) {
      if (hasSubstr(request.getURI(), path)) {
        return new RouteHandler(_format);
      }
    }

    return 0;
  }

 private:
  std::string _format;
};

#endif
