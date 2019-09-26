# cppgow: A C HTTP client and server

## What is it?

An HTTP server and client with a C and C++ API, implemented by binding the go http module.

## How to use it in C?

The API is very simple:

### Client

```C

void onResult(void*, int statusCode, void* payload, int payloadLength) {
    write(1, payload, payloadLength);
    exit(statusCode == 200 ? 0 : 1);
}

int main() {
  cppgowInitialize();
  struct CRequest req;
  req.method = "GET";
  req.url = "http://www.github.com";
  req.onResult = &onResult;
  req.userData = 0; req.payload = 0;req.headers = 0;
  cppgowRequest(&req);
  sleep(10);
}
```

Notes:

  - The CRequest object must be valid until the callback is invoked.
  - The callback is invoked from a random thread

### Server

```C
struct CServerResponse* ping(struct CServerRequest* req) {
  fprintf(stderr, "%s - %s - %s - %s\n", req->method, req->url, req->host, req->client);
  struct CServerResponse* resp = (struct CServerResponse*)malloc(sizeof(struct CServerResponse));
  resp->statusCode = 200;
  if (req->payload)
  {
    resp->payload = malloc(req->payloadLength);
    resp->payloadLength = req->payloadLength;
    memcpy(resp->payload, req->payload, resp->payloadLength);
  }
  return resp;
}

int main(int argc, char** argv)
{
  cppgowRegisterHandler("/ping", ping);
  cppgowListenAndServe(":8901");
  while(1)
    usleep(1000000);
}
```

Notes:

  - The CResponse pointer and payload will be freed by cppgow upon completion.
  - Your handler function will be called from a random thread

## How to use it in C++?

The API is even simpler:

### Client

```C++
cppgow::get("http://www.google.com", [](int statusCode, void* payload, int payloadLength) {
    if (statusCode != 200)
      std::cerr << "google is down" << std::endl;
});
```

### Server

```C++
#include <iostream>
#include <unistd.h>
#include "cppgowcxx.hh"


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
```
## Anything else I should know?

The API does not support streaming (yet?), so all bodies must fit in memory.

Since the heavy lifting is done by goroutines, your callbacks will be called
from a random thread.


## C++ Router

cppgow now includes codegen to generate routers based on annotation.

For instance:

```C++
//@route(/add/:a/:b)
int add(int a, int b, int c)
{ // c will be taken from query parameters
    return a+b+c;
}
```

Generate code with `./routegen.py mycppfile.cc > mycppfile_gen.cc`.

Include the generated file in your sources, and call `router::listenAndServe(":portnumber")` from main().

You can access the current request and response objects using `router::request()` and `router::response()`.

Things you should know about it:

  - The C++ *quote* parser *quote* is very fragile.
  - Supported argument types are integral types, float, double and std::string.
  - Your result value will be cast to a string using a stream.