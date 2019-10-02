#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "cppgow/libcppgow.h"



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

void* processor(void* rid)
{
    long requestId = (long)rid;
    cppgowWriteHeader(requestId, "X-Test", "foo");
    usleep(200000);
    cppgowWriteStatusCode(requestId, 200);
    usleep(200000);
    for (int i=0; i<10; ++i)
    {
        cppgowWriteData(requestId, "foo\n", 4);
        usleep(200000);
    }
    // this final call is mandatory for async requests
    cppgowWriteData(requestId, 0, 0);
}

struct CServerResponse* asyncRequest(struct CServerRequest* req) {
  fprintf(stderr, "%s - %s - %s - %s\n", req->method, req->url, req->host, req->client);
  struct CServerResponse* resp = (struct CServerResponse*)malloc(sizeof(struct CServerResponse));
  // To be able to send more headers asynchronously you must set statusCode and payload to 0
  resp->statusCode = 0;
  resp->payload = 0;
  resp->payloadLength = 0;
  pthread_t tid;
  pthread_create(&tid, 0, processor, (void*)req->requestId);
  return resp;
}


int main(int argc, char** argv)
{
  cppgowRegisterHandler("/ping", ping, 0, 0);
  cppgowRegisterHandler("/async", asyncRequest, 0, 1);
  cppgowListenAndServe(":8901");
  while(1)
    usleep(1000000);
}
