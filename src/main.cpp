#include <string.h>
#include <iostream>

#include "config.h"

#include "svr.h"


/*
 * -f,--file <filename>
 *
 */

int usage(const char *name)
{
    std::cout << std::endl << "Usage: " << std::endl;
    std::cout << name << std::endl;
    std::cout << "  -f,--file <config-filename>    default to 'config.yaml'" << std::endl;
    std::cout << std::endl ;

    return 1;
}

int runit() {
    Poco::UInt16 port = 9999;

    Poco::Net::ServerSocket socket(port);

    Poco::Net::HTTPServerParams *pParams = new Poco::Net::HTTPServerParams();
    //Sets the maximum number of queued connections.
    pParams->setMaxQueued(100);
    //Sets the maximum number of simultaneous threads available for this Server
    pParams->setMaxThreads(16);
    // Instanciate HandlerFactory
    Poco::Net::HTTPServer server(new HandlerFactory(), socket, pParams);

    server.start();

    sleep(100000);

    server.stop();

    return 0;

}

int main(int argc, char* argv[])
{
    auto i=1;
    const char *cfg_filename = "config.yaml";

    while(i<argc)
    {
        if(strcmp(argv[i],"-f")==0 || strcmp(argv[i],"--file")==0) {
            cfg_filename = argv[i + 1];
            i += 2;
        }
        else
        {
            return usage(argv[0]);
        }
    }

    uConfig::init(cfg_filename);

    std::cout << "Server: " << uConfig::get<std::string>("tip","server") << std::endl;
    std::cout << "Port: " << uConfig::get<int>("tip","port")+1  << std::endl;
    std::cout << "Keyfile: " << uConfig::get<std::string>("certificates","keyfile") << std::endl;

    runit();

    return 0;
}

