#pragma once

#include <string.h>

// request, status_code, body, bodyLength
typedef void (*RequestCallback)(void*, int, void*, int);
struct CRequest
{
  char* url;
  char* method;
  char* headers;
  void* payload;
  int   payloadLength;
  RequestCallback onResult;
  void* userData;
};

void invokeRequestCallback(RequestCallback rc, struct CRequest* req, int code, void* data, int len);




struct CServerRequest
{
  char* url;
  char* method;
  char* headers;
  char* host;
  char* client; // ip:port
  void* payload;
  int   payloadLength;
  void* userData;
};

struct CServerResponse
{
  char* headers;
  int   statusCode;
  void* payload;
  int   payloadLength;
};

// The server will free payload and headers and response upon copy
typedef struct CServerResponse* (*ServerCallback)(struct CServerRequest*);


struct CServerResponse* invokeServerCallback(ServerCallback sc, struct CServerRequest*);