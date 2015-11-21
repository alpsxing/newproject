#include <unistd.h>
#include "Utility.h"
#include "LogUtility.h"
#include "WebServer.h"
#include "HTTPBody.h"
#include <boost/network/protocol/http/server.hpp>

namespace http = boost::network::http;

struct WebServerProcessing;
typedef http::server<WebServerProcessing> server;

struct WebServerProcessing {
  void operator()(server::request const &request, server::response &response) {
    server::string_type ip = source(request);
    unsigned int port = request.source_port;
    std::string request_body = body(request);
    LogUtility::Log(LOG_LEVEL_DEBUG, "Received request: %s", request_body.c_str());
    WebServer::ProcessRequest(request_body);
    std::ostringstream data;
    data << "Hello, " << ip << ':' << port << '!';
    response = server::response::stock_reply(server::response::ok, data.str());
  }

  void log(...) {
    // do nothing
  }
};

void WebServer::Run(std::string addr, int port)
{
	LogUtility::Log(LOG_LEVEL_DEBUG, "Web server running at http://%s:%d",
					addr.c_str(), port);

	while (1)
	{
		try {
			WebServerProcessing handler;

			server::options options(handler);
			server server_(options.address(addr).port(numToString<int>(port)));
			server_.run();
		}
		catch (std::exception &e) {
			LogUtility::Log(LOG_LEVEL_ERROR, "Web server got exception");
		}

		LogUtility::Log(LOG_LEVEL_INFO, "Restart web server...");
		sleep(5);
	}
}

void WebServer::ProcessRequest(std::string request_body)
{
	HttpBody *resp;

	resp = HttpBody::CreateBody(request_body);
	if (!resp)
	{
		LogUtility::Log(LOG_LEVEL_WARN, "Invalid response");
		return;
	}

	LogUtility::Log(LOG_LEVEL_DEBUG, "Request body %s processed", resp->ToString().c_str());

	delete resp;
}
