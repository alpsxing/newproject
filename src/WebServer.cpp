#include "Utility.h"
#include "WebServer.h"
#include <boost/network/protocol/http/server.hpp>

namespace http = boost::network::http;

struct WebServerProcessing;
typedef http::server<WebServerProcessing> server;

struct WebServerProcessing {
  /*<< This is the function that handles the incoming request. >>*/
  void operator()(server::request const &request, server::response &response) {
    server::string_type ip = source(request);
    unsigned int port = request.source_port;
    std::string request_body = body(request);
    std::ostringstream data;
    data << "Hello, " << ip << ':' << port << '!';
    response = server::response::stock_reply(server::response::ok, data.str());
  }
  /*<< It's necessary to define a log function, but it's ignored in
       this example. >>*/
  void log(...) {
    // do nothing
  }
};

void WebServer::Run(std::string addr, int port)
{
	while (1)
	{
		try {
			WebServerProcessing handler;

			server::options options(handler);
			server server_(options.address(addr).port(numToString<int>(port)));
			server_.run();
		}
		catch (std::exception &e) {
			;
		}
	}
}
