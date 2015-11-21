#ifndef _WEB_SERVER_H_
#define _WEB_SERVER_H_

#include <string>

class WebServer
{
public:
	static void Run(std::string addr, int port);
	static void ProcessRequest(std::string request_body);
};

#endif
