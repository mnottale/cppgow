#include <string>
#include <regex>

#include "router.hh"

extern void router_register_routes();

thread_local cppgow::ServerResponse* current_response;
thread_local cppgow::ServerRequest* current_request;

namespace router
{
    struct Route
    {
        std::string prefix;
        std::string re;
        Handler handler;
    };
    static std::vector<Route> routes;
    static cppgow::ServerResponse processor(cppgow::ServerRequest const& req)
    {
        auto const& path = req.path;
        for(auto const& r: routes)
        {
            if (path.length() >= r.prefix.length() && path.substr(0, r.prefix.length()) == r.prefix)
            {
                std::regex re(r.re);
                std::smatch match;
                if (std::regex_match(path, match, re))
                {
                    std::vector<std::string> hits;
                    for (int i=1; i<match.size(); ++i)
                        hits.push_back(match[i]);
                    try
                    {
                        current_request = (cppgow::ServerRequest*) &req;
                        return r.handler(req, hits);
                    }
                    catch (std::exception const& e)
                    {
                        cppgow::ServerResponse response;
                        response.statusCode = 500;
                        response.payload = cppgow::BufferView(strdup(e.what()), strlen(e.what()));
                        return response;
                    }
                }
            }
        }
        cppgow::ServerResponse response;
        response.statusCode = 404;
        return response;
    }
    void registerRoute(std::string const& prefix, std::string const& re, Handler handler)
    {
        Route r{prefix, re, handler};
        routes.push_back(r);
    }
    void listenAndServe(std::string const& hostPort)
    {
        router_register_routes();
        cppgow::registerRoute("/", processor);
        cppgow::listenAndServe(hostPort);
    }
    cppgow::ServerRequest& request()
    {
        return *current_request;
    }
    cppgow::ServerResponse& response()
    {
        return *current_response;
    }
    void setResponse(cppgow::ServerResponse* ptr)
    {
        current_response = ptr;
    }
}