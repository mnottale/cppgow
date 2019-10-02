#include <string>
#include <regex>
#include <iostream>

#include "cppgow/router.hh"


thread_local cppgow::ServerResponseWriter* current_response;
thread_local cppgow::ServerRequest* current_request;

namespace router
{
    struct Route
    {
        std::string method;
        std::string prefix;
        std::string re;
        Handler handler;
    };
    static std::vector<Route>& routes()
    {
        static std::vector<Route> routes;
        return routes;
    }
    static void processor(cppgow::ServerRequest const& req, cppgow::ServerResponseWriter& writer)
    {
        auto const& path = req.path;
        //std::cerr << "REQUEST " << req.method << " " << req.path << std::endl;
        for(auto const& r: routes())
        {
            //std::cerr << "scan " << r.method << " " << r.prefix << " " << r.re << std::endl;
            if (!r.method.empty() && r.method != req.method)
                continue;
            if (path.length() >= r.prefix.length() && path.substr(0, r.prefix.length()) == r.prefix)
            {
                std::regex re(r.re);
                std::smatch match;
                if (std::regex_match(path, match, re))
                {
                    std::vector<std::string> hits;
                    for (unsigned long i=1; i<match.size(); ++i)
                        hits.push_back(match[i]);
                    try
                    {
                        current_request = (cppgow::ServerRequest*) &req;
                        current_response = &writer;
                        const_cast<cppgow::ServerRequest&>(req).parameters = hits;
                        r.handler(req, writer);
                        return;
                    }
                    catch (std::exception const& e)
                    {
                        writer.close(500, e.what());
                        return;
                    }
                }
            }
        }
        writer.close(404);
    }
    void registerRoute(std::string const& prefix, std::string const& re, Handler handler)
    {
        Route r{std::string(), prefix, re, handler};
        routes().push_back(r);
    }
    void registerRoute(std::string const& method, std::string const& prefix, std::string const& re, Handler handler)
    {
        Route r{method, prefix, re, handler};
        routes().push_back(r);
    }
    void listenAndServe(std::string const& hostPort)
    {
        cppgow::registerRoute("/", processor);
        cppgow::listenAndServe(hostPort);
    }
    cppgow::ServerRequest& request()
    {
        return *current_request;
    }
    void setResponseWriter(cppgow::ServerResponseWriter* ptr)
    {
        current_response = ptr;
    }
    cppgow::ServerResponseWriter responseWriterTake()
    {
        return std::move(*current_response);
    }
    cppgow::ServerResponseWriter& responseWriterAccess()
    {
        return *current_response;
    }
}