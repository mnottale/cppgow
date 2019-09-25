# cppgow: A C HTTP client and server

## What is it?

An HTTP server and client with a C API, implemented by binding the go http module.

## How to use it?

The API is very simple:

### Client

```
void onResult(void*, int statusCode, void* payload, int payloadLength) {
    write(0, payload, payloadLength);
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

## Anything else I should know?

The API does not support streaming (yet?), so all bodies must fit in memory.

Since the heavy lifting is done by goroutines, your callbacks will be called
from a random thread.

## Why is it called cppgow?

I'll make a C++ binding on top of the C API eventually.


