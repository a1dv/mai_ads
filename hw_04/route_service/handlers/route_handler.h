#ifndef USERouteHandler_H
#define USERouteHandler_H

#include <fstream>
#include <iostream>
#include <utility>
#include "Poco/DateTimeFormat.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Exception.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/ThreadPool.h"
#include "Poco/Timestamp.h"
#include "Poco/URI.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/ServerApplication.h"

#include <Poco/Base64Decoder.h>
#include <Poco/Base64Encoder.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Parser.h>

using Poco::DateTimeFormat;
using Poco::DateTimeFormatter;
using Poco::ThreadPool;
using Poco::Timestamp;
using Poco::Net::HTMLForm;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerParams;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::NameValueCollection;
using Poco::Net::ServerSocket;
using Poco::Util::Application;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionCallback;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;

#include "../../database/route.h"
#include "../../helper.h"

class RouteHandler : public HTTPRequestHandler {
 public:
  explicit RouteHandler(std::string  format) : _format(std::move(format)) {}

  static std::optional<std::string> do_get(const std::string& url, const std::string& identity) {
    std::string string_result;

    try {
      Poco::URI uri(url);
      Poco::Net::HTTPClientSession s(uri.getHost(), uri.getPort());
      Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET,
                                     uri.toString());
      request.setVersion(Poco::Net::HTTPMessage::HTTP_1_1);
      request.setContentType("application/json");
      request.set("Authorization", identity);
      request.set("Accept", "application/json");
      request.setKeepAlive(true);

      s.sendRequest(request);

      Poco::Net::HTTPResponse response;
      std::istream& rs = s.receiveResponse(response);

      while (rs) {
        char c{};
        rs.read(&c, 1);
        if (rs)
          string_result += c;
      }

      if (response.getStatus() != 200)
        return {};
    } catch (Poco::Exception& ex) {
      std::cout << "exception:" << ex.what() << std::endl;
      return {};
    }

    return string_result;
  }

  static std::optional<std::string> do_get(const std::string& url,
                                    const std::string& login,
                                    const std::string& password) {
    std::string token = login + ":" + password;
    std::ostringstream os;
    Poco::Base64Encoder b64in(os);
    b64in << token;
    b64in.close();
    std::string identity = "Basic " + os.str();

    return do_get(url, identity);
  }

  static bool authRequest(HTTPServerRequest& request, long& user_id) {
    HTMLForm form(request, request.stream());
    std::string scheme;
    std::string info;
    std::string login, password;

    request.getCredentials(scheme, info);
    if (scheme == "Basic") {
      get_identity(info, login, password);
      std::cout << "login:" << login << std::endl;
      std::cout << "password:" << password << std::endl;
      std::string host = "localhost";
      std::string url;

      if (std::getenv("SERVICE_HOST") != nullptr) {
        host = std::getenv("SERVICE_HOST");
      }
      url = "http://" + host + ":8080/auth";

      std::optional<std::string> string_result = do_get(url, login, password);

      if (string_result) {
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(*string_result);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
        user_id = object->getValue<long>("id");

        return true;
      }
    }

    return false;
  }


  void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override {
    HTMLForm form(request, request.stream());
    long user_id;

    if (!authRequest(request, user_id)) {
      response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_UNAUTHORIZED);
      response.setChunkedTransferEncoding(true);
      response.setContentType("application/json");
      Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
      root->set("type", "/errors/unauthorized");
      root->set("title", "Unauthorized");
      root->set("detail", "invalid login or password");
      root->set("instance", "/user");
      std::ostream& ostr = response.send();
      Poco::JSON::Stringifier::stringify(root, ostr);

      return;
    }

    if (hasSubstr(request.getURI(), "/search") &&
      (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET)) {
        handleGet( response, user_id);
        return;
    }

    if (hasSubstr(request.getURI(), "/add") &&
          (request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST)) {
      handleAdd(request, response);
      return;
    }

    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");
    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
    root->set("type", "/errors/not_found");
    root->set("title", "Internal exception");
    root->set("detail", "request not found");
    root->set("instance", "/route");
    std::ostream& ostr = response.send();
    Poco::JSON::Stringifier::stringify(root, ostr);
  }



  static void handleAdd(HTTPServerRequest& request, HTTPServerResponse& response) {
    HTMLForm form(request, request.stream());
    if (form.has("from") and form.has("to")) {
      database::Route route;
      route.from() = form.get("from");
      route.to() = form.get("to");
      
      route.create();

      response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
      response.setChunkedTransferEncoding(true);
      response.setContentType("application/json");
      Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
      std::ostream& ostr = response.send();
      Poco::JSON::Stringifier::stringify(root, ostr);
      return;
    }

    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");
    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
    root->set("type", "/errors/bad_request");
    root->set("title", "Missing fields in create request");
    root->set("detail", "Missing fields in create request");
    root->set("instance", "/route");
    std::ostream& ostr = response.send();
    Poco::JSON::Stringifier::stringify(root, ostr);
  }

  static void handleGet(HTTPServerResponse& response, long user_id) {
        auto res = database::Route::get_by_user_id(user_id);

        Poco::JSON::Array arr;

        for(auto a : res) {
            arr.add(a.toJSON());
        }

      response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
      response.setChunkedTransferEncoding(true);
      response.setContentType("application/json");
      std::ostream& ostr = response.send();
      Poco::JSON::Stringifier::stringify(arr, ostr);
 }

 private:
  std::string _format;
};
#endif
