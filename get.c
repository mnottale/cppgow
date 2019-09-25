#include <stdio.h>
#include <unistd.h>

#include "cppgow.h"


void onResult(void* req, int sc, void* data, int dataLen)
{
  if (sc == -1)
  {
    fprintf(stderr, "ERROR:\n");
    write(2, data, dataLen);
    fprintf(stderr, "\n");
    exit(2);
  }
  write(1, data, dataLen);
  write(1, "\n", 1);
  exit(sc/100 == 2 ? 0 : 3);
}
int main(int argc, char** argv)
{
  cppgowInitialize();
  struct CRequest req;
  req.url = argv[1];
  req.method = "GET";
  req.headers = 0;
  req.userData = 0;
  req.onResult = &onResult;
  cppgowRequest(&req);
  usleep(100000000);
  printf("TIMEOUT");
  return 1;
}