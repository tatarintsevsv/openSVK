#include <iostream>
#include <string>
#include "svkcompose.h"

using namespace std;



int main(int argc,char* argv[])
{
    if(argc<2)
        return -1;
    svkCompose* compose = new svkCompose(argv[1]);
    compose->run();
    delete compose;
    return 0;
}
