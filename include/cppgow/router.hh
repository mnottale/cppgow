#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iostream> //debug

#include "cppgow/cppgowcxx.hh"

namespace router
{
    template<typename T>
    T cast(std::string const& str)
    {
        std::stringstream ss(str);
        T v;
        ss >> v;
        return v;
    }

    inline std::string cast(std::string const& str)
    {
        return str;
    }

    template<typename T>
    void response(cppgow::ServerResponse& resp, T const& v)
    {
        std::stringstream ss;
        ss << v;
        ss.flush();
        std::string st(ss.str());
        resp.payload = cppgow::BufferView(strdup(st.c_str()), st.length());
    }

    using Handler = std::function<cppgow::ServerResponse(cppgow::ServerRequest, std::vector<std::string>)>;
    void setAsync(bool mode);
    void registerRoute(std::string const& prefix, std::string const& re, Handler handler);
    void registerRoute(std::string const& method, std::string const& prefix, std::string const& re, Handler handler);
    void listenAndServe(std::string const& hostPort, bool asyncRoutes = false);

    cppgow::ServerRequest& request();
    cppgow::ServerResponse& response();
    std::vector<std::string>& requestParams();
    void setResponse(cppgow::ServerResponse* ptr);
}