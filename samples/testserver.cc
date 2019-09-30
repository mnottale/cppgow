#include <iostream>
#include <unistd.h>
#include "cppgow/cppgowcxx.hh"


int main(int argc, char** argv)
{
    cppgow::registerRoute("/", [](cppgow::ServerRequest const& req) -> cppgow::ServerResponse {
            std::cerr << req.method << " on " << req.url << " from " << req.client << std::endl;
            for (auto const& h: req.headers)
                std::cerr << "  " << h.first << " set to " << h.second << std::endl;
            std::cerr << "path: " << req.path << std::endl;
            for (auto const& h: req.query)
                std::cerr << "  " << h.first << " set to " << h.second << std::endl;
            cppgow::ServerResponse resp;
            resp.statusCode = 200;
            return resp;
    });
    cppgow::listenAndServe(":8901");
    while (true)
        usleep(1000000);
}