#include <string>
#include <iostream>
#include <boost/program_options.hpp>
#include "LogUtility.h"
#include "WebServer.h"
#include "ControlChannel.h"
#include "DataChannel.h"
#include "CollectChannel.h"

namespace po = boost::program_options;

std::string _web_server_addr = "127.0.0.1";
int _web_server_port = 5001;

static int ProcessOptions(int argc, char *argv[])
{
    try {
        po::options_description options("Allowed options");
        options.add_options()
            ("help", "produce help message")
            ("addr", po::value<std::string>(), "web server binding address")
            ("port", po::value<int>(), "web server port")
            ("debug", po::value<int>(), "set debug level")
            ("log", po::value<int>(), "set log level")
        ;

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, options), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << options << "\n";
            return 1;
        }

        if (vm.count("addr")) {
        	_web_server_addr = vm["addr"].as<std::string>();
        }

        if (vm.count("port")) {
        	_web_server_port = vm["port"].as<int>();
        }

        if (vm.count("debug")) {
            std::cout << "debug level was set to "
                 << vm["debug"].as<int>() << ".\n";
            LogUtility::SetStdoutLevel(vm["debug"].as<int>());
        }

        if (vm.count("log")) {
            std::cout << "log level was set to "
                 << vm["log"].as<int>() << ".\n";
            LogUtility::SetLogLevel(vm["log"].as<int>());
        }
    }
    catch(std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch(...) {
        std::cerr << "Exception of unknown type!\n";
    }

    return 0;
}

int main(int argc, char **argv)
{
	if(ProcessOptions(argc, argv) != 0)
    {
	    return 1;
	}

	CONTROL_INSTANCE->Start();
	DATA_INSTANCE->Start();
	COLLECT_INSTANCE->Start();

	WebServer::Run(_web_server_addr, _web_server_port);

	CONTROL_INSTANCE->Stop();
	DATA_INSTANCE->Stop();
	COLLECT_INSTANCE->Stop();

    return 0;
}
