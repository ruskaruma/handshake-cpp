#include "webserver/server.hpp"
#include "webserver/log.hpp"
#include <cstdlib>
int main(int argc,char** argv)
{
    int port=8080;
    if(argc>=2) port=std::atoi(argv[1]);
    webserver::WebSocketServer server(port);
    server.on_open=[](int fd)
    {
        webserver::log("connection opened (fd="+std::to_string(fd)+")");
    };
    server.on_close=[](int fd)
    {
        webserver::log("connection closed (fd="+std::to_string(fd)+")");
    };
    server.on_message=[](int fd,const std::vector<uint8_t>& data,bool is_text)
    {
        if(is_text)
        {
            std::string msg(data.begin(),data.end());
            webserver::log("fd "+std::to_string(fd)+" -> text message: "+msg);
        }
        else
        {
            webserver::log("fd "+std::to_string(fd)+" -> binary message ("+std::to_string(data.size())+" bytes)");
        }
    };
    server.run();
    return 0;
}
