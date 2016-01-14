#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "lib_linux_config.h"
#include "buffer.h"
using namespace std;
using namespace lib_linux;

int main(int argc, char *argv[])
{
    Buffer buffer(2);

    assert(buffer.realloc(1) == 4);
    assert(buffer.realloc(1) == 8);
    assert(buffer.realloc(128) == 256);
    assert(buffer.realloc(500) == 1024);
    assert(buffer.realloc() == 2048);

    return 0;
}
