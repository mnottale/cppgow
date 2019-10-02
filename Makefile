

all: get testserver testservercxx testrouter

get: libcppgow.so samples/get.c
	gcc -g -o get -Iinclude samples/get.c -L. -lcppgow

testserver: libcppgow.so samples/testserver.c
	gcc -pthread -g -o testserver -Iinclude samples/testserver.c -L. -lcppgow

testservercxx: libcppgowcxx.so samples/testserver.cc
	g++ -g -o testservercxx -Iinclude samples/testserver.cc -L. -lcppgowcxx -lcppgow

libcppgow_invoke.a: src/cppgow_invoke.c
	gcc -g -Iinclude -c -o cppgow_invoke.o src/cppgow_invoke.c
	ar cru libcppgow_invoke.a cppgow_invoke.o

libcppgow.so: libcppgow_invoke.a src/cppgow.go include/cppgow/cppgowc.h
	go build -o libcppgow.so -buildmode=c-shared src/cppgow.go
	mv libcppgow.h include/cppgow/

libcppgowcxx.so: libcppgow.so src/cppgowcxx.cc include/cppgow/cppgowcxx.hh
	g++ -g -Wall -Wsign-compare -fPIC -o libcppgowcxx.so -shared -Iinclude src/cppgowcxx.cc -L. -lcppgow


samples/testrouter_gen.cc: samples/testrouter.cc routegen.py
	./routegen.py samples/testrouter.cc > samples/testrouter_gen.cc

testrouter: include/cppgow/router.hh src/router.cc samples/testrouter.cc samples/testrouter_gen.cc
	g++ -Wall -g -o testrouter -Iinclude samples/testrouter.cc src/router.cc -L. -lcppgowcxx -lcppgow

clean:
	rm libcppgow.so libcppgowcxx.so libcppgow_invoke.a cppgow_invoke.o
