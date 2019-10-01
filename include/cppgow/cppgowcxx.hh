#pragma once

#include "cppgow/cppgow.h"

#include <unordered_map>
#include <string>
#include <functional>
#include <vector>

namespace cppgow
{
    class BufferView
    {
    public:
        void* data;
        int size;
        BufferView() : data(nullptr), size(0) {}
        BufferView(void* d, int s) : data(d), size(s) {}
    };
    using Headers = std::unordered_map<std::string, std::string>;
    using RequestCallback = std::function<void(int, void*, int)>;
    void initialize();
    /// Gereric HTTP request. `payload` must live until callback is invoked.
    void request(std::string const& method, std::string const& url, const Headers* headers, BufferView payload, RequestCallback callback);
    void get(std::string const& url, RequestCallback callback);
    void get(std::string const& url, Headers const& headers, RequestCallback callback);
    void post(std::string const& url, BufferView payload, RequestCallback callback);
    void post(std::string const& url, BufferView payload, Headers const& headers, RequestCallback callback);
    void put(std::string const& url, BufferView payload, RequestCallback callback);
    void put(std::string const& url, BufferView payload, Headers const& headers, RequestCallback callback);

    struct ServerRequest
    {
        std::string method;
        std::string url;
        std::string path;
        std::string host;
        std::vector<std::string> parameters; //< filled by router
        Headers query;
        Headers headers;
        std::string client;
        BufferView payload;
        long requestId;
    };
    struct ServerResponse
    {
        Headers headers;
        int statusCode;
        BufferView payload;
    };
    
    class ServerResponseWriter
    {
    public:
        ServerResponseWriter(long requestId);
        ~ServerResponseWriter();
        ServerResponseWriter(ServerResponseWriter const& b) = delete;
        ServerResponseWriter(ServerResponseWriter && b);
        ServerResponseWriter& operator = (ServerResponseWriter const& b) = delete;
        ServerResponseWriter& operator = (ServerResponseWriter&& b);

        void setStatusCode(int sc);
        void setHeader(std::string const& key, std::string const& value);
        void write(const void* data, int size);
        void write(std::string const& data);
        void close();
        void close(int sc, std::string const& payload = "");
        bool valid();
    private:
        void _checkValidOpen();
        long _requestId;
        bool _closed;
    };

    using RouteHandler = std::function<ServerResponse(ServerRequest const&)>;
    using RouteHandlerAsync = std::function<void(ServerRequest const&, ServerResponseWriter&)>; 
    

    void registerRoute(std::string const& path, RouteHandler handler);
    void registerRoute(std::string const& path, RouteHandlerAsync handler);
    void listenAndServe(std::string const& hostPort);
}