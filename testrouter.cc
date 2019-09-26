#include <string>
#include <unistd.h>

#include "router.hh"

//@route(/add/:a/:b)
int add(int a, int b, int c)
{
    return a+b+c;
}

//@route(/concat/:a/:b)
std::string concat(std::string a, std::string b)
{
    auto token = router::request().headers["X-Token"];
    router::response().headers["X-Token"] = token;
    return a+b;
}

#include "testrouter_gen.cc"

int main()
{
    router::listenAndServe(":8901");
    while (true)
        usleep(1000000);
}