#include <stdio.h>
#include <unistd.h>

#include "cppgow.h"



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