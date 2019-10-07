/* cppgow, Copyright (C) 2019, Matthieu Nottale
 * see COPYING file for licensing information
*/

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
    void response(cppgow::ServerResponseWriter& resp, T const& v)
    {
        if (resp.valid())
        {
            std::stringstream ss;
            ss << v;
            ss.flush();
            std::string st(ss.str());
            resp.close(200, st);
        }
    }

    using Handler = cppgow::RouteHandlerAsync;

    void registerRoute(std::string const& prefix, std::string const& re, Handler handler);
    void registerRoute(std::string const& method, std::string const& prefix, std::string const& re, Handler handler);
    void listenAndServe(std::string const& hostPort);

    cppgow::ServerRequest& request();
    cppgow::ServerResponseWriter responseWriterTake(); //< steal
    cppgow::ServerResponseWriter& responseWriterAccess(); //< peek
    void setResponseWriter(cppgow::ServerResponseWriter* ptr);
}