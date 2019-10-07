/* cppgow, Copyright (C) 2019, Matthieu Nottale
 * see COPYING file for licensing information
*/

#include "cppgow/cppgowcxx.hh"

namespace cppgow
{
    void initialize()
    {
        cppgowInitialize();
    }
    static char* headersPack(Headers const& headers)
    {
        std::string res;
        for (auto const& h: headers)
            res += h.first + ":" + h.second + "\n";
        return strdup(res.c_str());
    }
    static void onRequestResult(void* vreq, int statusCode, void* payload, int payloadLength)
    {
        CRequest* req = (CRequest*)vreq;
        RequestCallback* cbPtr = (RequestCallback*)req->userData;
        (*cbPtr)(statusCode, payload, payloadLength);
        
        free(req->url);
        free(req->method);
        free(req->headers);
        delete cbPtr;
        delete req;
    }
    void request(std::string const& method, std::string const& url, const Headers* headers, BufferView payload, RequestCallback callback)
    {
        CRequest* req = new CRequest();
        req->url = strdup(url.c_str());
        req->method = strdup(method.c_str());
        if (headers)
            req->headers = headersPack(*headers);
        else
            req->headers = nullptr;
        req->payload = payload.data;
        req->payloadLength = payload.size;
        req->userData = new RequestCallback(callback);
        req->onResult = onRequestResult;
        cppgowRequest(req);
    }
    void get(std::string const& url, RequestCallback callback)
    {
        request("GET", url, nullptr, BufferView(), callback);
    }
    void get(std::string const& url, Headers const& headers, RequestCallback callback)
    {
        request("GET", url, &headers, BufferView(), callback);
    }
    void post(std::string const& url, BufferView payload, RequestCallback callback)
    {
        request("POST", url, nullptr, payload, callback);
    }
    void post(std::string const& url, BufferView payload, Headers const& headers, RequestCallback callback)
    {
        request("POST", url, &headers, payload, callback);
    }
    void put(std::string const& url, BufferView payload, RequestCallback callback)
    {
        request("PUT", url, nullptr, payload, callback);
    }
    void put(std::string const& url, BufferView payload, Headers const& headers, RequestCallback callback)
    {
        request("PUT", url, &headers, payload, callback);
    }

    static Headers parseHeaders(std::string const& h)
    {
        Headers res;
        unsigned long p = 0;
        while (p < h.length())
        {
            auto pendl = h.find_first_of('\n', p);
            if (pendl == h.npos)
                pendl = h.length();
            auto psep = h.find_first_of(':', p);
            res.emplace(h.substr(p, psep-p), h.substr(psep+1, pendl-psep-1));
            p = pendl + 1;
        }
        return res;
    }
    static std::string urlDecode(std::string const& qq)
    {
        std::string res;
        for (unsigned long p=0; p<qq.length(); ++p)
        {
            char c = qq[p];
            if (c == '%' &&  p < qq.length()-2)
            {
                char dgt[3] = {qq[p+1], qq[p+2], 0};
                res += (char)strtol(dgt, 0, 16);
                p += 2;
            }
            else
                res += c;
        }
        return res;
    }
    static void parseQuery(std::string const& qq, std::string& outPath, Headers& outQuery)
    {
        std::string q = urlDecode(qq);
        auto sep = q.find_first_of('?');
        if (sep == q.npos)
        {
            outPath = q;
            return;
        }
        outPath = q.substr(0, sep);
        q = q.substr(sep+1, std::string::npos);
        unsigned long p = 0;
        while (p < q.length())
        {
            auto pendl = q.find_first_of('&', p);
            if (pendl == q.npos)
                pendl = q.length();
            auto psep = q.find_first_of('=', p);
            if (psep > pendl)
                psep = pendl-1;
            std::string pkey = q.substr(p, psep-p);
            std::string pval = q.substr(psep+1, pendl-psep-1);
            outQuery.emplace(pkey, pval);
            p = pendl+1;
        }
    }
    static CServerResponse* onSyncRouteCalled(CServerRequest* creq)
    {
        ServerRequest req;
        req.method = creq->method;
        req.url = creq->url;
        req.host = creq->host;
        req.client = creq->client;
        req.payload = BufferView(creq->payload, creq->payloadLength);
        req.headers = parseHeaders(creq->headers);
        req.requestId = creq->requestId;
        parseQuery(req.url, req.path, req.query);
        RouteHandler* handler = (RouteHandler*)creq->userData;
        ServerResponse resp = (*handler)(req);
        CServerResponse* cresp = (CServerResponse*)malloc(sizeof(CServerResponse));
        memset(cresp, 0, sizeof(CServerResponse));
        cresp->statusCode = resp.statusCode;
        cresp->headers = headersPack(resp.headers);
        cresp->payload = resp.payload.data;
        cresp->payloadLength = resp.payload.size;
        return cresp;
    }

    static CServerResponse* onAsyncRouteCalled(CServerRequest* creq)
    {
        ServerRequest req;
        req.method = creq->method;
        req.url = creq->url;
        req.host = creq->host;
        req.client = creq->client;
        req.payload = BufferView(creq->payload, creq->payloadLength);
        req.headers = parseHeaders(creq->headers);
        req.requestId = creq->requestId;
        parseQuery(req.url, req.path, req.query);
        RouteHandlerAsync* handler = (RouteHandlerAsync*)creq->userData;
        ServerResponseWriter srw(req.requestId);
        (*handler)(req, srw);
        CServerResponse* cresp = (CServerResponse*)malloc(sizeof(CServerResponse));
        memset(cresp, 0, sizeof(CServerResponse));
        cresp->statusCode = 0;
        cresp->payload = 0;
        cresp->payloadLength = 0;
        return cresp;
    }

    void registerRoute(std::string const& path, RouteHandler handler)
    {
        cppgowRegisterHandler((char*)path.c_str(), onSyncRouteCalled, new RouteHandler(handler), 0);
    }

    void registerRoute(std::string const& path, RouteHandlerAsync handler)
    {
        cppgowRegisterHandler((char*)path.c_str(), onAsyncRouteCalled, new RouteHandlerAsync(handler), 1);
    }

    void listenAndServe(std::string const& hostPort)
    {
        cppgowListenAndServe((char*)hostPort.c_str());
    }

    ServerResponseWriter::ServerResponseWriter(long rid)
    : _requestId(rid)
    , _closed(false)
    {}

    ServerResponseWriter::ServerResponseWriter(ServerResponseWriter&& b)
    : _requestId(b._requestId)
    , _closed(b._closed)
    {
        b._requestId = 0;
    }

    ServerResponseWriter& ServerResponseWriter::operator=(ServerResponseWriter&& b)
    {
        _requestId = b._requestId;
        _closed = b._closed;
        b._requestId = 0;
        return *this;
    }

    void ServerResponseWriter::setStatusCode(int sc)
    {
        _checkValidOpen();
        cppgowWriteStatusCode(_requestId, sc);
    }

    void ServerResponseWriter::setHeader(std::string const& k, std::string const& v)
    {
        _checkValidOpen();
        cppgowWriteHeader(_requestId, (char*)k.c_str(), (char*)v.c_str());
    }

    void ServerResponseWriter::write(const void* data, int size)
    {
        _checkValidOpen();
        cppgowWriteData(_requestId, (void*)data, size);
    }

    void ServerResponseWriter::write(std::string const& data)
    {
        write(data.data(), data.size());
    }

    void ServerResponseWriter::close()
    {
        _checkValidOpen();
        cppgowWriteData(_requestId, 0, 0);
        _closed = true;
    }

    void ServerResponseWriter::close(int sc, std::string const& payload)
    {
        _checkValidOpen();
        setStatusCode(sc);
        if (payload.length() != 0)
            write(payload);
        close();
    }

    void ServerResponseWriter::_checkValidOpen()
    {
        if (_requestId == 0)
            throw std::runtime_error("ServerResponseWriter has no associated request");
        if (_closed)
            throw std::runtime_error("ServerResponseWriter is closed");
    }

    ServerResponseWriter::~ServerResponseWriter()
    {
        if (_requestId != 0 && !_closed)
            close();
    }

    bool ServerResponseWriter::valid()
    {
        return _requestId != 0 && !_closed;
    }
}
