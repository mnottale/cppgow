package main


// #cgo LDFLAGS: -L. -lcppgow_invoke
// #cgo CFLAGS: -g
// #include <stdlib.h>
// #include "cppgowc.h"
import "C"
import "unsafe"
import "bytes"
import "io"
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
  go cppgowSyncRequest(request);
}

func bounceErr(request *C.struct_CRequest, err error) {
  serr := err.Error()
  cslen := len(serr)
  cs := C.CString(serr)
  C.invokeRequestCallback(request.onResult, request, -1, unsafe.Pointer(cs), C.int(cslen))
  C.free(unsafe.Pointer(cs))
}

//export cppgowSyncRequest
func cppgowSyncRequest(request *C.struct_CRequest) {
  var payload io.Reader = nil
  if request.payload != nil {
    data := C.GoBytes(request.payload, request.payloadLength)
      payload = bytes.NewBuffer(data)
  }
  req, err := http.NewRequest(C.GoString(request.method), C.GoString(request.url), payload)
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
  resp.Body.Close()
  if err != nil {
    bounceErr(request, err)
    return
  }
  C.invokeRequestCallback(request.onResult, request, C.int(resp.StatusCode), unsafe.Pointer(&body[0]), C.int(len(body)));
}

//export cppgowRegisterHandler
func cppgowRegisterHandler(route *C.char, handler C.ServerCallback) {
  http.HandleFunc(C.GoString(route), func(w http.ResponseWriter, r *http.Request) {
    creq := C.struct_CServerRequest {}
      creq.url = C.CString(r.URL.String())
      creq.method = C.CString(r.Method)
      headers := ""
      for k, v := range r.Header {
        for _, vv := range v {
          headers += k + ":" + vv + "\n"
        }
      }
      creq.headers = C.CString(headers)
      creq.host = C.CString(r.Host)
      creq.client = C.CString(r.RemoteAddr)
      body, err := ioutil.ReadAll(r.Body)
      if err != nil {
        return
      }
      creq.payload = C.CBytes(body) // we are forced to copy here...
      creq.payloadLength = C.int(len(body))
      csr := C.invokeServerCallback(handler, &creq)
      if csr == nil {
        w.WriteHeader(500)
        return
      }
      hd := w.Header()
      if csr.headers != nil {
        hs := C.GoString(csr.headers)
        hss := strings.Split(hs, "\n")
          for _, kv := range hss {
            kvs := strings.SplitN(kv, ":", 2)
            hd.Add(kvs[0], kvs[1])
          }
      }
      w.WriteHeader(int(csr.statusCode))
      if csr.payload != nil {
        w.Write(C.GoBytes(csr.payload, csr.payloadLength))
      }
      // cleanup
      C.free(unsafe.Pointer(creq.url))
      C.free(unsafe.Pointer(creq.method))
      C.free(unsafe.Pointer(creq.headers))
      C.free(unsafe.Pointer(creq.host))
      C.free(unsafe.Pointer(creq.client))
      C.free(unsafe.Pointer(creq.payload))
      C.free(unsafe.Pointer(csr.headers))
      C.free(unsafe.Pointer(csr.payload))
      C.free(unsafe.Pointer(csr))
  })
}
//export cppgowListenAndServe
func cppgowListenAndServe(port *C.char) {
  gport := C.GoString(port)
  go func() {
    http.ListenAndServe(gport, nil)
  }()
}

func main() {
}