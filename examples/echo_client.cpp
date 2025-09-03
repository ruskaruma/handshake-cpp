#include "webserver/webclient.hpp"
#include<iostream>
int main()
{
    try
    {
        webserver::WebSocketClient client("127.0.0.1", 8080, "/");
        client.send_text("Hello, Server!");
        auto frame = client.recv_message();
        if (frame)
        {
            std::cout << "Received: "
                      << std::string(frame->payload.begin(), frame->payload.end())
                      << std::endl;
        }
    }
    catch(const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
