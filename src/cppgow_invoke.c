#include "cppgow/cppgowc.h"

void invokeRequestCallback(RequestCallback rc,struct CRequest* req, int code, void* data, int len)
{
  (rc)(req, code, data, len);
}

struct CServerResponse* invokeServerCallback(ServerCallback sc, struct CServerRequest* req)
{
  return sc(req);
}