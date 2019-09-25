package main


// #cgo LDFLAGS: -L. -lcppgow_invoke
// #cgo CFLAGS: -g
// #include <stdlib.h>
// #include "cppgowc.h"
import "C"
import "unsafe"
import "io/ioutil"
import "net/http"
import "strings"

var client *http.Client;

//export cppgowInitialize
func cppgowInitialize() {
  client = &http.Client{}
}
  
//export cppgowRequest
func cppgowRequest(request *C.struct_CRequest) {
  go syncRequest(request);
}

func bounceErr(request *C.struct_CRequest, err error) {
  serr := err.Error()
  cslen := len(serr)
  cs := C.CString(serr)
  C.invokeRequestCallback(request.onResult, request, -1, unsafe.Pointer(cs), C.int(cslen))
  C.free(unsafe.Pointer(cs))
}

func syncRequest(request *C.struct_CRequest) {
  req, err := http.NewRequest(C.GoString(request.method), C.GoString(request.url), nil)
  if err != nil {
    bounceErr(request, err)
    return
  }
  if request.headers != nil {
    hs := C.GoString(request.headers)
    hss := strings.Split(hs, "\n")
    for _, kv := range hss {
      kvs := strings.SplitN(kv, ":", 2)
      req.Header.Add(kvs[0], kvs[1])
    }
  }
  resp, err := client.Do(req)
  if err != nil {
    bounceErr(request, err)
    return
  }
  body, err := ioutil.ReadAll(resp.Body)
  if err != nil {
    bounceErr(request, err)
    return
  }
  C.invokeRequestCallback(request.onResult, request, C.int(resp.StatusCode), unsafe.Pointer(&body[0]), C.int(len(body)));
}

func main() {
}