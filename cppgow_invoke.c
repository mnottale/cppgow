#include "cppgowc.h"

void invokeRequestCallback(RequestCallback rc,struct CRequest* req, int code, void* data, int len)
{
  (rc)(req, code, data, len);
}
