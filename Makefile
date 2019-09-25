

all: get

get: cppgow.so get.c
	gcc -g -o get -I. get.c ./cppgow.so

libcppgow_invoke.a: cppgow_invoke.c
	gcc -g -c -o cppgow_invoke.o cppgow_invoke.c
	ar cru libcppgow_invoke.a cppgow_invoke.o

cppgow.so: libcppgow_invoke.a cppgow.go cppgowc.h
	go build -o cppgow.so -buildmode=c-shared cppgow.go

clean:
	rm cppgow.so libcppgow_invoke.a cppgow_invoke.o