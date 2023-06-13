#include <iostream>
#include <string>
#include "svkmain.h"

using namespace std;



int main(int argc,char* argv[])
{
    svkMain* svk = new svkMain();
    svk->run();
    delete svk;
    return 0;
}
