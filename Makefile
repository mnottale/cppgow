

all: get testserver testservercxx testrouter

get: cppgow.so get.c
	gcc -g -o get -I. get.c ./cppgow.so

testserver: cppgow.so testserver.c
	gcc -pthread -g -o testserver -I. testserver.c ./cppgow.so

testservercxx: cppgowcxx.so testserver.cc
	g++ -g -o testservercxx -I. testserver.cc ./cppgowcxx.so ./cppgow.so

libcppgow_invoke.a: cppgow_invoke.c
	gcc -g -c -o cppgow_invoke.o cppgow_invoke.c
	ar cru libcppgow_invoke.a cppgow_invoke.o

cppgow.so: libcppgow_invoke.a cppgow.go cppgowc.h
	go build -o cppgow.so -buildmode=c-shared cppgow.go

cppgowcxx.so: cppgow.so cppgowcxx.cc cppgowcxx.hh
	g++ -fPIC -o cppgowcxx.so -shared -I. cppgowcxx.cc ./cppgow.so


testrouter_gen.cc: testrouter.cc routegen.py
	./routegen.py testrouter.cc > testrouter_gen.cc

testrouter: router.hh router.cc testrouter.cc testrouter_gen.cc
	g++ -o testrouter -I. testrouter.cc router.cc ./cppgowcxx.so ./cppgow.so

clean:
	rm cppgow.so libcppgow_invoke.a cppgow_invoke.o