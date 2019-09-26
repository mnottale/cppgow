#include "cppgow.h"

#include <unordered_map>
#include <string>
#include <functional>

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
        Headers query;
        Headers headers;
        std::string client;
        BufferView payload;
    };
    struct ServerResponse
    {
        Headers headers;
        int statusCode;
        BufferView payload;
    };
    using RouteHandler = std::function<ServerResponse(ServerRequest const&)>;
 
    void registerRoute(std::string const& path, RouteHandler handler);
    void listenAndServe(std::string const& hostPort);
}