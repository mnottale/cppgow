#include <string.h>

// request, status_code, body, bodyLength
typedef void (*RequestCallback)(void*, int, void*, int);
struct CRequest
{
  char* url;
  char* method;
  char* headers;
  RequestCallback onResult;
  void* userData;
};

void invokeRequestCallback(RequestCallback rc, struct CRequest* req, int code, void* data, int len);
