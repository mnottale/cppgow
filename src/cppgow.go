/*

cppgow C HTTP client and server
Copyright (C) 2019  Matthieu Nottale

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

package main


// #cgo LDFLAGS: -L.. -L../../../build -lcppgow_invoke
// #cgo CFLAGS: -g -O2 -I../include
// #include <stdlib.h>
// #include "cppgow/cppgowc.h"
import "C"
import "unsafe"
import "bytes"
import "io"
import "io/ioutil"
import "net/http"
import "strings"
import "sync"

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

type AsyncRequest struct {
    data []byte
    headerKey string
    headerValue string
    statusCode int
    finish bool
}

var (
   writersMutex *sync.Mutex = &sync.Mutex{}
   writers map[int]chan AsyncRequest = make(map[int]chan AsyncRequest)
   nextWriterId int = 1
)

//export cppgowWriteHeader
func cppgowWriteHeader(uid C.long, hk *C.char, hv *C.char) {
    var c chan AsyncRequest
    writersMutex.Lock()
    c, ok := writers[int(uid)]
    writersMutex.Unlock()
    if !ok {
        return
    }
    c <- AsyncRequest { headerKey: C.GoString(hk), headerValue: C.GoString(hv)}
}

//export cppgowWriteStatusCode
func cppgowWriteStatusCode(uid C.long, sc C.int) {
    var c chan AsyncRequest
    writersMutex.Lock()
    c, ok := writers[int(uid)]
    writersMutex.Unlock()
    if !ok {
        return
    }
    c <- AsyncRequest { statusCode: int(sc) }
}

//export cppgowWriteData
func cppgowWriteData(uid C.long, data unsafe.Pointer, length C.int) {
    var c chan AsyncRequest
    writersMutex.Lock()
    c, ok := writers[int(uid)]
    writersMutex.Unlock()
    if !ok {
        return
    }
    if data == nil {
        c <- AsyncRequest{ finish: true}
    } else {
        c <- AsyncRequest{ data: C.GoBytes(data, length)}
    }
}

//export cppgowWriteAndClose
func cppgowWriteAndClose(uid C.long, statusCode C.int, data *C.char) {
     var c chan AsyncRequest
    writersMutex.Lock()
    c, ok := writers[int(uid)]
    writersMutex.Unlock()
    if !ok {
        return
    }
    if data == nil {
        c <- AsyncRequest{ statusCode: int(statusCode), finish: true}
    } else {
        c <- AsyncRequest{ statusCode: int(statusCode), data: []byte(C.GoString(data)), finish: true}
    }
}

//export cppgowRegisterHandler
func cppgowRegisterHandler(route *C.char, handler C.ServerCallback, userData unsafe.Pointer, isAsync C.int) {
  http.HandleFunc(C.GoString(route), func(w http.ResponseWriter, r *http.Request) {
      flusher, canFlush := w.(http.Flusher)
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
      creq.userData = userData
      var c chan AsyncRequest = nil
      var uid int = 0
      if isAsync != 0 {
          writersMutex.Lock()
          uid = nextWriterId
          nextWriterId += 1
          c = make(chan AsyncRequest, 10)
          writers[uid] = c
          writersMutex.Unlock()
      }
      creq.requestId = C.long(uid)
      csr := C.invokeServerCallback(handler, &creq)
      if csr == nil {
        w.WriteHeader(500)
        return
      }
      hd := w.Header()
      if csr.headers != nil {
        hs := C.GoString(csr.headers)
        if hs != "" {
            hss := strings.Split(hs, "\n")
            for _, kv := range hss {
                kvs := strings.SplitN(kv, ":", 2)
                if len(kvs) == 2 {
                  hd.Add(kvs[0], kvs[1])
                } else {
                  hd.Add(kvs[0], "")
                }
            }
        }
      }
      if csr.statusCode != 0 || csr.payload != nil {
          w.WriteHeader(int(csr.statusCode))
          if csr.payload != nil {
              w.Write(C.GoBytes(csr.payload, csr.payloadLength))
          }
      }
      if isAsync != 0 {
          for {
              asr := <- c
              if asr.headerKey != "" {
                  w.Header().Add(asr.headerKey, asr.headerValue)
              }
              if asr.statusCode != 0 {
                  w.WriteHeader(asr.statusCode)
              }
              if asr.data != nil {
                  w.Write(asr.data)
                  if canFlush {
                      flusher.Flush()
                  }
              }
              if asr.finish {
                  break
              }
          }
          writersMutex.Lock()
          delete(writers, uid)
          writersMutex.Unlock()
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