#include "cppgowcxx.hh"

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
        int p = 0;
        while (p < h.length())
        {
            int pendl = h.find_first_of('\n', p);
            if (pendl == h.npos)
                pendl = h.length();
            int psep = h.find_first_of(':', p);
            res.emplace(h.substr(p, psep-p), h.substr(psep+1, pendl-psep-1));
            p = pendl + 1;
        }
        return res;
    }
    static std::string urlDecode(std::string const& qq)
    {
        std::string res;
        for (int p=0; p<qq.length(); ++p)
        {
            char c = qq[p];
            if (c == '%' &&  p < qq.length()-2)
            {
                char dgt[3] = {qq[p+1], qq[p+2], 0};
                res += (char)strtol(dgt, 0, 16);
            }
            else
                res += c;
        }
        return res;
    }
    static void parseQuery(std::string const& qq, std::string& outPath, Headers& outQuery)
    {
        std::string q = urlDecode(qq);
        int sep = q.find_first_of('?');
        if (sep == q.npos)
        {
            outPath = q;
            return;
        }
        outPath = q.substr(0, sep);
        q = q.substr(sep+1, std::string::npos);
        int p = 0;
        while (p < q.length())
        {
            int pendl = q.find_first_of('&', p);
            if (pendl == q.npos)
                pendl = q.length();
            int psep = q.find_first_of('=', p);
            if (psep > pendl)
                psep = pendl-1;
            std::string pkey = q.substr(p, psep-p);
            std::string pval = q.substr(psep+1, pendl-psep-1);
            outQuery.emplace(pkey, pval);
            p = pendl+1;
        }
    }
    static CServerResponse* onRouteCalled(CServerRequest* creq)
    {
        ServerRequest req;
        req.method = creq->method;
        req.url = creq->url;
        req.host = creq->host;
        req.client = creq->client;
        req.payload = BufferView(creq->payload, creq->payloadLength);
        req.headers = parseHeaders(creq->headers);
        parseQuery(req.url, req.path, req.query);
        RouteHandler* handler = (RouteHandler*)creq->userData;
        ServerResponse resp = (*handler)(req);
        CServerResponse* cresp = (CServerResponse*)malloc(sizeof(CServerResponse));
        cresp->statusCode = resp.statusCode;
        cresp->headers = headersPack(resp.headers);
        cresp->payload = resp.payload.data;
        cresp->payloadLength = resp.payload.size;
        return cresp;
    }

    void registerRoute(std::string const& path, RouteHandler handler)
    {
        cppgowRegisterHandler((char*)path.c_str(), onRouteCalled, new RouteHandler(handler));
    }

    void listenAndServe(std::string const& hostPort)
    {
        cppgowListenAndServe((char*)hostPort.c_str());
    }
}