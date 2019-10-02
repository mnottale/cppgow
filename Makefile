

all: get testserver testservercxx testrouter

get: cppgow.so samples/get.c
	gcc -g -o get -Iinclude samples/get.c ./cppgow.so

testserver: cppgow.so samples/testserver.c
	gcc -pthread -g -o testserver -Iinclude samples/testserver.c ./cppgow.so

testservercxx: cppgowcxx.so samples/testserver.cc
	g++ -g -o testservercxx -Iinclude samples/testserver.cc ./cppgowcxx.so ./cppgow.so

libcppgow_invoke.a: src/cppgow_invoke.c
	gcc -g -Iinclude -c -o cppgow_invoke.o src/cppgow_invoke.c
	ar cru libcppgow_invoke.a cppgow_invoke.o

cppgow.so: libcppgow_invoke.a src/cppgow.go include/cppgow/cppgowc.h
	go build -o cppgow.so -buildmode=c-shared src/cppgow.go
	mv cppgow.h include/cppgow/

cppgowcxx.so: cppgow.so src/cppgowcxx.cc include/cppgow/cppgowcxx.hh
	g++ -g -Wall -Wsign-compare -fPIC -o cppgowcxx.so -shared -Iinclude src/cppgowcxx.cc ./cppgow.so


samples/testrouter_gen.cc: samples/testrouter.cc routegen.py
	./routegen.py samples/testrouter.cc > samples/testrouter_gen.cc

testrouter: include/cppgow/router.hh src/router.cc samples/testrouter.cc samples/testrouter_gen.cc
	g++ -Wall -g -o testrouter -Iinclude samples/testrouter.cc src/router.cc ./cppgowcxx.so ./cppgow.so

clean:
	rm cppgow.so libcppgow_invoke.a cppgow_invoke.o
